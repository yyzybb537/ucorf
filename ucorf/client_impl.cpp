#include "client_impl.h"
#include "error.h"
#include "dispatcher.h"
#include "logger.h"
#include "message.h"
#include "net_transport.h"

namespace ucorf
{
    ClientImpl::ClientImpl()
        : opt_(new Option), dispatcher_(new RobinDispatcher),
        head_factory_(&UcorfHead::Factory), default_srv_finder_(new ServerFinder),
        tp_factory_([]{ return static_cast<ITransportClient*>(new NetTransportClient); })
    {
        default_srv_finder_->SetConnectedCb(boost::bind(&ClientImpl::OnConnected, this, _1, _2));
        default_srv_finder_->SetReceiveCb(boost::bind(&ClientImpl::OnReceiveData, this, _1, _2, _3, _4));
        default_srv_finder_->SetDisconnectedCb(boost::bind(&ClientImpl::OnDisconnected, this, _1, _2, _3));
        default_srv_finder_->SetOption(opt_);
    }

    ClientImpl::~ClientImpl()
    {
    }

    ClientImpl& ClientImpl::SetOption(boost::shared_ptr<Option> opt)
    {
        opt_ = opt;
        if (default_srv_finder_)
            default_srv_finder_->SetOption(opt_);

        for (auto &finder : srv_finders_)
            finder->SetOption(opt_);
        return *this;
    }

    ClientImpl& ClientImpl::SetDispatcher(std::unique_ptr<IDispatcher> && dispatcher)
    {
        dispatcher_.reset();
        dispatcher_.swap(dispatcher);
        return *this;
    }

    ClientImpl& ClientImpl::SetHeaderFactory(HeaderFactory const& head_factory)
    {
        head_factory_ = head_factory;
        return *this;
    }

    ClientImpl& ClientImpl::SetServerFinder(std::unique_ptr<ServerFinder> && srv_finder)
    {
        if (default_srv_finder_) default_srv_finder_.reset();

        srv_finder->SetConnectedCb(boost::bind(&ClientImpl::OnConnected, this, _1, _2));
        srv_finder->SetReceiveCb(boost::bind(&ClientImpl::OnReceiveData, this, _1, _2, _3, _4));
        srv_finder->SetDisconnectedCb(boost::bind(&ClientImpl::OnDisconnected, this, _1, _2, _3));
        srv_finder->SetOption(opt_);
        if (!url_.empty())
            srv_finder->Init(url_, tp_factory_);
        srv_finders_.emplace_back(std::move(srv_finder));
        return *this;
    }

    ClientImpl& ClientImpl::SetTransportFactory(TransportFactory const& factory)
    {
        tp_factory_ = factory;
        return *this;
    }

    ClientImpl& ClientImpl::SetUrl(std::string const& url)
    {
        url_ = url;
        if (default_srv_finder_)
            default_srv_finder_->Init(url_, tp_factory_);

        for (auto &finder : srv_finders_)
            finder->Init(url_, tp_factory_);
        return *this;
    }

    boost_ec ClientImpl::Call(std::string const& service_name,
            std::string const& method_name,
            IMessage *request, IMessage *response)
    {
        if (wnd_size_ > opt_->request_wnd_size)
            return MakeUcorfErrorCode(eUcorfErrorCode::ec_req_wnd_full);

        boost::shared_ptr<ITransportClient> tp = dispatcher_->Get(service_name, method_name, request);
        if (!tp) {
            bool ok = false;
            boost_ec last_ec;
            if (default_srv_finder_) {
                boost_ec ec = default_srv_finder_->ReConnect();
                if (ec) {
                    last_ec = ec;
                    ucorf_log_error("connect to %s error: %s", url_.c_str(), ec.message().c_str());
                } else
                    ok = true;
            }

            for (auto &finder : srv_finders_) {
                boost_ec ec = finder->ReConnect();
                if (ec) {
                    last_ec = ec;
                    ucorf_log_error("connect to %s error: %s", url_.c_str(), ec.message().c_str());
                } else
                    ok = true;
            }
            if (!ok)
                return last_ec;

            tp = dispatcher_->Get(service_name, method_name, request);
        }

        if (!tp || !tp->IsEstab()) {
            return MakeUcorfErrorCode(eUcorfErrorCode::ec_no_estab);
        }

        IHeaderPtr header = head_factory_();
        std::size_t msg_id = ++msg_id_;
        header->SetId(msg_id);
        header->SetType(response ? eHeaderType::request : eHeaderType::oneway_request);
        header->SetFollowBytes(request->ByteSize());
        header->SetService(service_name);
        header->SetMethod(method_name);
        std::vector<char> buf;
        buf.resize(header->ByteSize() + request->ByteSize());
        header->Serialize(&buf[0], header->ByteSize());
        request->Serialize(&buf[header->ByteSize()], request->ByteSize());
        if (!response) {
            co_chan<boost_ec> cc(1);
            tp->Send(std::move(buf), [=](boost_ec const& ec) { cc << ec; });
            boost_ec ec;
            cc >> ec;
            return ec;
        } else {
            std::unique_lock<co_mutex> channel_lock(channel_mtx_);
            auto it_1 = channels_.find(tp.get());
            if (channels_.end() == it_1)
                return MakeUcorfErrorCode(eUcorfErrorCode::ec_no_estab);
            auto chan_grp = it_1->second;
            channel_lock.unlock();

            auto chan = RspChan(1);
            int map_idx = msg_id & (e_chan_group_count - 1);

            RspChanMap &chan_map = chan_grp->maps[map_idx];
            co_mutex &mtx = chan_grp->mtxs[map_idx];

            std::unique_lock<co_mutex> map_lock(mtx);
            if (chan_grp->closed[map_idx])
                return MakeUcorfErrorCode(eUcorfErrorCode::ec_no_estab);
            chan_map[msg_id] = chan;
            map_lock.unlock();

            tp->Send(&buf[0], buf.size(), [=](boost_ec const& ec){
                        if (ec) {
                            chan.TryPush(ec);
                        }
                    });
            ++wnd_size_;

            ResponseData rsp;
            if (opt_->rcv_timeout_ms) {
                if (!chan.TimedPop(rsp, std::chrono::milliseconds(opt_->rcv_timeout_ms)))
                    return MakeUcorfErrorCode(eUcorfErrorCode::ec_rcv_timeout);
            } else
                chan >> rsp;

            --wnd_size_;

            {
                std::unique_lock<co_mutex> map_lock(mtx);
                chan_map.erase(msg_id);
            }

            if (rsp.ec)
                return rsp.ec;

            if (rsp.data.empty() || !response->Parse(&rsp.data[0], rsp.data.size()))
                return MakeUcorfErrorCode(eUcorfErrorCode::ec_parse_error);
        }

        return boost_ec();
    }

    void ClientImpl::OnConnected(boost::shared_ptr<ITransportClient> tp, SessId sess_id)
    {
        dispatcher_->Add(tp);

        std::unique_lock<co_mutex> channel_lock(channel_mtx_);
        channels_[tp.get()].reset(new RspChanGroup);
    }
    void ClientImpl::OnDisconnected(boost::shared_ptr<ITransportClient> tp, SessId sess_id, boost_ec const& ec)
    {
        dispatcher_->Del(tp);

        std::unique_lock<co_mutex> channel_lock(channel_mtx_);
        auto it_1 = channels_.find(tp.get());
        if (channels_.end() == it_1) return ;
        auto chan_grp = it_1->second;
        channels_.erase(it_1);
        channel_lock.unlock();

        for (int i = 0; i < e_chan_group_count; ++i)
        {
            std::unique_lock<co_mutex> map_lock(chan_grp->mtxs[i]);
            chan_grp->closed[i] = true;
            auto &map = chan_grp->maps[i];
            for (auto &kv : map)
                kv.second.TryPush(ec);
        }
    }

//    static std::string to_14_hex(const char* data, size_t len)
//    {
//        static const char hex[] = "0123456789abcdef";
//        len = std::min<size_t>(len, 28);
//        std::string str;
//        for (size_t i = 0; i < len; ++i) {
//            str += hex[(unsigned char)data[i] >> 4];
//            str += hex[(unsigned char)data[i] & 0xf];
//            str += ' ';
//        }
//        return str;
//    }

    size_t ClientImpl::OnReceiveData(boost::shared_ptr<ITransportClient> tp, SessId sess_id, const char* data, size_t bytes)
    {
        size_t consume = 0;
        const char* buf = data;
        size_t len = bytes;

        int yield_c = 0;
        while (consume < bytes)
        {
            IHeaderPtr header = head_factory_();
            size_t head_len = header->Parse(buf, len);
            if (!head_len) {
//                ucorf_log_verb("header parse error, len = %u, bin: %s", (unsigned)len, to_14_hex(buf, len).c_str());
                break;
            }

            size_t follow_bytes = header->GetFollowBytes();
            if (head_len + follow_bytes > len) {
//                ucorf_log_verb("follow bytes too less, head_len = %u, follow_bytes = %u, len = %u", (unsigned)head_len, (unsigned)follow_bytes, (unsigned)len);
                break;
            }

            OnResponse(tp, header, buf + head_len, follow_bytes);

            consume += head_len + follow_bytes;
            buf = data + consume;
            len = bytes - consume;

            if ((++yield_c & 0xff) == 0)
                co_yield;
        }

        if (yield_c <= 0xff)
            co_yield;

//        ucorf_log_debug("consume %u bytes from %u bytes", (unsigned)consume, (unsigned)bytes);
        return consume;
    }

    void ClientImpl::OnResponse(boost::shared_ptr<ITransportClient> tp, IHeaderPtr header, const char* data, size_t bytes)
    {
//        ucorf_log_debug("receive response. srv=%s, method=%s, msgid=%llu",
//                header->GetService().c_str(), header->GetMethod().c_str(), (unsigned long long)header->GetId());

        std::size_t msg_id = header->GetId();

        std::unique_lock<co_mutex> channel_lock(channel_mtx_);
        auto it_1 = channels_.find(tp.get());
        if (channels_.end() == it_1) {
            ucorf_log_warn("discard response because connection was disconnected. srv=%s, method=%s, msgid=%llu",
                    header->GetService().c_str(), header->GetMethod().c_str(), (unsigned long long)header->GetId());
            return ;
        }
        auto chan_grp = it_1->second;
        channel_lock.unlock();

        int map_idx = msg_id & (e_chan_group_count - 1);

        RspChanMap &chan_map = chan_grp->maps[map_idx];
        co_mutex &mtx = chan_grp->mtxs[map_idx];

        std::unique_lock<co_mutex> map_lock(mtx);
        auto it_2 = chan_map.find(msg_id);
        if (chan_map.end() == it_2) {
            ucorf_log_warn("discard response because stub was timeout. srv=%s, method=%s, msgid=%llu",
                    header->GetService().c_str(), header->GetMethod().c_str(), (unsigned long long)header->GetId());
            return ;
        }
        auto chan = it_2->second;
        map_lock.unlock();

        ResponseData rsp;
        rsp.header = header;
        rsp.data.resize(bytes);
        memcpy(&rsp.data[0], data, bytes);
        chan.TryPush(rsp);
    }

} //namespace ucorf


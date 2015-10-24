#include "client.h"
#include "error.h"
#include "dispatcher.h"

namespace ucorf
{
    Client::Client()
    {}

    Client::~Client()
    {
    }

    Client& Client::SetOption(Option const& opt)
    {
        opt_ = opt;
        return *this;
    }

    Client& Client::SetDispatcher(std::unique_ptr<IDispatcher> && dispatcher)
    {
        dispatcher_.reset();
        dispatcher_.swap(dispatcher);
        return *this;
    }

    Client& Client::SetHeaderFactory(HeaderFactory const& head_factory)
    {
        head_factory_ = head_factory;
        return *this;
    }

    Client& Client::SetServerFinder(std::unique_ptr<ServerFinder> && srv_finder)
    {
        srv_finder_.reset();
        srv_finder_.swap(srv_finder);
        srv_finder_->SetConnectedCb(boost::bind(&Client::OnConnected, this, _1, _2));
        srv_finder_->SetReceiveCb(boost::bind(&Client::OnReceiveData, this, _1, _2, _3, _4));
        srv_finder_->SetDisconnectedCb(boost::bind(&Client::OnDisconnected, this, _1, _2, _3));
        if (!url_.empty())
            srv_finder_->Init(url_, tp_factory_);
        return *this;
    }

    Client& Client::SetTransportFactory(TransportFactory const& factory)
    {
        tp_factory_ = factory;
        return *this;
    }

    Client& Client::SetUrl(std::string const& url)
    {
        url_ = url;
        if (srv_finder_)
            srv_finder_->Init(url_, tp_factory_);
        return *this;
    }

    boost_ec Client::Call(std::string const& service_name,
            std::string const& method_name,
            IMessage *request, IMessage *response)
    {
        boost::shared_ptr<ITransportClient> tp = dispatcher_->Get(service_name, method_name, request);
        if (!tp) {
            boost_ec ec = srv_finder_->ReConnect();
            if (ec) return ec;
            tp = dispatcher_->Get(service_name, method_name, request);
        }

        if (!tp->IsEstab()) {
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
            tp->Send(&buf[0], buf.size());
        } else {
            auto it = channels_[tp.get()].insert(std::make_pair(msg_id, RspChan(1))).first;
            auto chan = it->second;
            tp->Send(&buf[0], buf.size(), [=](boost_ec const& ec){
                        if (ec) {
                            chan.TryPush(ec);
                        } else if (opt_.rcv_timeout_ms) {
                            // start rcv timer.
                            co_timer_add(std::chrono::milliseconds(opt_.rcv_timeout_ms), [chan]{
                                    chan.TryPush(MakeUcorfErrorCode(eUcorfErrorCode::ec_rcv_timeout));
                                });
                        }
                    });
            ResponseData rsp;
            chan >> rsp;
            if (rsp.ec)
                return rsp.ec;

            if (rsp.data.empty() || !response->Parse(&rsp.data[0], rsp.data.size()))
                return MakeUcorfErrorCode(eUcorfErrorCode::ec_parse_error);
        }

        return boost_ec();
    }

    void Client::OnConnected(boost::shared_ptr<ITransportClient> tp, SessId sess_id)
    {
        dispatcher_->Add(tp);
    }
    void Client::OnDisconnected(boost::shared_ptr<ITransportClient> tp, SessId sess_id, boost_ec const& ec)
    {
        dispatcher_->Del(tp);

        auto it_1 = channels_.find(tp.get());
        if (channels_.end() == it_1) return ;

        auto &tp_table = it_1->second;
        for (auto &kv : tp_table)
        {
            kv.second.TryPush(ec);
        }

        channels_.erase(tp.get());
    }

    size_t Client::OnReceiveData(boost::shared_ptr<ITransportClient> tp, SessId sess_id, const char* data, size_t bytes)
    {
        size_t consume = 0;
        const char* buf = data;
        size_t len = bytes;

        while (consume < bytes)
        {
            IHeaderPtr header = head_factory_();
            size_t head_len = header->Parse(buf, len);
            if (!head_len) break;

            size_t follow_bytes = header->GetFollowBytes();
            if (head_len + follow_bytes > len) break;

            OnResponse(tp, header, buf + head_len, follow_bytes);

            consume += head_len + follow_bytes;
            buf = data + consume;
            len = bytes - consume;
        }

        return consume;
    }

    void Client::OnResponse(boost::shared_ptr<ITransportClient> tp, IHeaderPtr header, const char* data, size_t bytes)
    {
        std::size_t msg_id = header->GetId();
        auto it_1 = channels_.find(tp.get());
        if (channels_.end() == it_1) return ;

        auto &tp_table = it_1->second;
        auto it_2 = tp_table.find(msg_id);
        if (tp_table.end() == it_2) return ;

        auto &chan = it_2->second;
        ResponseData rsp;
        rsp.header = header;
        rsp.data.resize(bytes);
        memcpy(&rsp.data[0], data, bytes);
        chan.TryPush(rsp);
        tp_table.erase(it_2);
    }

} //namespace ucorf


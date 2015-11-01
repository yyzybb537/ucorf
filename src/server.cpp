#include "server.h"
#include "message.h"
#include "service.h"
#include "logger.h"
#include "zookeeper.h"
#include <boost/algorithm/string.hpp>

namespace ucorf
{
    Server::Server()
        : opt_(new Option), register_(new ZookeeperRegister)
    {
    }

    Server::~Server()
    {
        register_->Unregister();
    }

    Server& Server::BindTransport(std::unique_ptr<ITransportServer> && transport)
    {
        if (!opt_->transport_opt.empty())
            transport->SetOption(opt_->transport_opt);

        transport->SetReceiveCb(boost::bind(&Server::OnReceiveData,
                    this, transport.get(), _1, _2, _3));
        transport->SetConnectedCb(boost::bind(&Server::OnConnected,
                    this, transport.get(), _1));
        transport->SetDisconnectedCb(boost::bind(&Server::OnDisconnected,
                    this, transport.get(), _1, _2));
        transports_.push_back(std::move(transport));
        return *this;
    }

    Server& Server::SetOption(boost::shared_ptr<Option> opt)
    {
        opt_ = opt;
        for (auto &p:transports_)
            p->SetOption(opt_);
        return *this;
    }

    Server& Server::SetHeaderFactory(HeaderFactory const& head_factory)
    {
        head_factory_ = head_factory;
        return *this;
    }

    Server& Server::SetRegister(boost::shared_ptr<IServerRegister> reg)
    {
        register_->Unregister();
        register_ = reg;
        return *this;
    }

    bool Server::RegisterTo(std::string const& url)
    {
        for (auto &tp : transports_) {
            std::string addr = tp->LocalUrl();
            if (!register_->Register(url, addr))
                return false;
        }

        return true;
    }

    bool Server::RegisterService(std::shared_ptr<IService> service)
    {
        std::string name = service->name();
        return services_.insert(std::make_pair(name, service)).second;
    }

    void Server::RemoveService(std::string const& service_name)
    {
        services_.erase(service_name);
    }

    void Server::OnConnected(ITransportServer *tp, SessId sess_id)
    {
        (void)tp;
        (void)sess_id;
    }
    void Server::OnDisconnected(ITransportServer *tp, SessId sess_id, boost_ec const& ec)
    {
        (void)tp;
        (void)sess_id;
        (void)ec;
    }

    size_t Server::OnReceiveData(ITransportServer *tp, SessId sess_id, const char* data, size_t bytes)
    {
        size_t consume = 0;
        const char* buf = data;
        size_t len = bytes;

        int yield_c = 0;
        while (consume < bytes)
        {
            IHeaderPtr header = head_factory_();
            size_t head_len = header->Parse(buf, len);
            if (!head_len) break;

            size_t follow_bytes = header->GetFollowBytes();
            if (head_len + follow_bytes > len) break;

            Session sess = {sess_id, tp, header};
            if (!DispatchMsg(sess, buf + head_len, follow_bytes))
                return -1;

            consume += head_len + follow_bytes;
            buf = data + consume;
            len = bytes - consume;

            if ((++yield_c & 0xff) == 0)
                co_yield;
        }

        if (yield_c <= 0xff)
            co_yield;
        return consume;
    }

    bool Server::DispatchMsg(Session sess, const char* data, size_t bytes)
    {
        std::string const& srv_name = sess.header->GetService();
        auto it = services_.find(srv_name);
        if (services_.end() == it) return false;

        auto &service = it->second;
        std::unique_ptr<IMessage> response(service->CallMethod(sess.header->GetMethod(), data, bytes));
        if (!response) return true;

        // reply
        if (sess.header->GetType() != eHeaderType::oneway_request) {
            sess.header->SetType(eHeaderType::response);
            std::vector<char> buf;
            buf.resize(sess.header->ByteSize() + response->ByteSize());
            sess.header->Serialize(&buf[0], sess.header->ByteSize());
            if (!response->Serialize(&buf[sess.header->ByteSize()], response->ByteSize())) {
                ucorf_log_warn("response serialize error. srv=%s, method=%s, msgid=%llu",
                        sess.header->GetService().c_str(), sess.header->GetMethod().c_str(),
                        (unsigned long long)sess.header->GetId());
                return true;
            }

            sess.transport->Send(sess.sess, &buf[0], buf.size(), [sess](boost_ec const& ec) {
                    if (ec)
                        ucorf_log_warn("response send error: %s. srv=%s, method=%s, msgid=%llu",
                            ec.message().c_str(), sess.header->GetService().c_str(),
                            sess.header->GetMethod().c_str(), (unsigned long long)sess.header->GetId());
                    });
        }

        return true;
    }

} //namespace ucorf

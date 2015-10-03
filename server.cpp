#include "server.h"
#include "message.h"

namespace ucorf
{
    Server& Server::Listen(ITransportServer *transport)
    {
        transport->SetReceiveCb(boost::bind(&Server::OnReceiveData,
                    this, transport, _1, _2, _3));
        transports_.push_back(transport);
        return *this;
    }

    Server& Server::SetHeaderFactory(HeaderFactory const& head_factory)
    {
        head_factory_ = head_factory;
        return *this;
    }

    Server& Server::SetMessageFactory(MessageFactory const& msg_factory)
    {
        msg_factory_ = msg_factory;
        return *this;
    }

    Server& Server::RegisterTo(std::string const& url)
    {
        // TODO: zookeeper
        return *this;
    }

    bool Server::RegisterService(IService* service)
    {
        std::string name = service->name();
        return services_.insert(std::make_pair(name, service)).second;
    }

    void Server::RemoveService(std::string const& service_name)
    {
        services_.erase(service_name);
    }

    size_t Server::OnReceiveData(ITransportServer *tp, SessId sess_id, const char* data, size_t bytes)
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

            Session sess = {sess_id, tp, header};
            if (!DispatchMsg(sess, buf + head_len, follow_bytes))
                return -1;

            consume += head_len + follow_bytes;
            buf = data + consume;
            len = bytes - consume;
        }

        return consume;
    }

    bool Server::DispatchMsg(Session sess, const char* data, size_t bytes)
    {
        std::string const& srv_name = sess.header->GetService();
        auto it = services_.find(srv_name);
        if (services_.end() == it) return false;

        IService *service = it->second;
        IMessage *response = service->CallMethod(sess.header->GetMethod(), data, bytes);
        if (!response) return true;

        // reply
        if (sess.header->GetType() != eHeaderType::oneway_request) {
            sess.header->SetType(eHeaderType::response);
            std::vector<char> buf;
            buf.resize(sess.header->ByteSize() + response->ByteSize());
            sess.header->Serialize(&buf[0], sess.header->ByteSize());
            response->Serialize(&buf[sess.header->ByteSize()], response->ByteSize());
            sess.transport->Send(sess.sess, &buf[0], buf.size());
        }

        return true;
    }

} //namespace ucorf

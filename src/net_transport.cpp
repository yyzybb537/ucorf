#include "net_transport.h"

namespace ucorf
{
    // server
    void NetTransportServer::Shutdown()
    {
        s_.Shutdown();
    }
    void NetTransportServer::SetReceiveCb(OnReceiveF const& cb)
    {
        s_.SetReceiveCb([=](::network::SessionId sess, const char* data, size_t bytes)
                {
                    return cb(boost::any(sess), data, bytes);
                });
    }
    void NetTransportServer::SetConnectedCb(OnConnectedF const& cb)
    {
        s_.SetConnectedCb([=](::network::SessionId sess)
                {
                    cb(boost::any(sess));
                });
    }
    void NetTransportServer::SetDisconnectedCb(OnDisconnectedF const& cb)
    {
        s_.SetDisconnectedCb([=](::network::SessionId sess, ::network::boost_ec const& ec)
                {
                    cb(boost::any(sess), ec);
                });
    }

    boost_ec NetTransportServer::Listen(std::string const& url)
    {
        return s_.goStart(url);
    }
    void NetTransportServer::Send(SessId id, const void* data, size_t bytes, OnSndF const& cb)
    {
        ::network::SessionId &sess = ::boost::any_cast<::network::SessionId&>(id);
        s_.GetProtocol()->Send(sess, data, bytes, cb);
    }

    // client
    void NetTransportClient::Shutdown()
    {
        c_.Shutdown();
    }
    void NetTransportClient::SetReceiveCb(OnReceiveF const& cb)
    {
        c_.SetReceiveCb([=](::network::SessionId sess, const char* data, size_t bytes)
                {
                    return cb(boost::any(sess), data, bytes);
                });
    }
    void NetTransportClient::SetConnectedCb(OnConnectedF const& cb)
    {
        c_.SetConnectedCb([=](::network::SessionId sess)
                {
                    cb(boost::any(sess));
                });
    }
    void NetTransportClient::SetDisconnectedCb(OnDisconnectedF const& cb)
    {
        c_.SetDisconnectedCb([=](::network::SessionId sess, ::network::boost_ec const& ec)
                {
                    cb(boost::any(sess), ec);
                });
    }

    boost_ec NetTransportClient::Connect(std::string const& url)
    {
        return c_.Connect(url);
    }
    void NetTransportClient::Send(const void* data, size_t bytes, OnSndF const& cb)
    {
        c_.Send(data, bytes, cb);
    }
    bool NetTransportClient::IsEstab()
    {
        auto proto = c_.GetProtocol();
        if (!proto) return false;
        return c_.GetProtocol()->IsEstab(c_.GetSessId());
    }

} //namespace ucorf

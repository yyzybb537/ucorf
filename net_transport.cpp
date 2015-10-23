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
        s_.SetReceiveCb([=](SessionId sess, const char* data, size_t bytes)
                {
                    cb(boost::any(sess), data, bytes);
                });
    }
    void NetTransportServer::SetConnectedCb(OnConnectedF const& cb)
    {
        s_.SetConnectedCb([=](SessionId sess)
                {
                    cb(boost::any(sess));
                });
    }
    void NetTransportServer::SetDisconnectedCb(OnDisconnectedF const& cb)
    {
        s_.SetDisconnectedCb([=](SessionId sess, ::network::boost_ec const& ec)
                {
                    cb(boost::any(sess), ec);
                });
    }

    bool NetTransportServer::Listen(std::string const& url)
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
        c_.SetReceiveCb([=](SessionId sess, const char* data, size_t bytes)
                {
                    cb(boost::any(sess), data, bytes);
                });
    }
    void NetTransportClient::SetConnectedCb(OnConnectedF const& cb)
    {
        c_.SetConnectedCb([=](SessionId sess)
                {
                    cb(boost::any(sess));
                });
    }
    void NetTransportClient::SetDisconnectedCb(OnDisconnectedF const& cb)
    {
        c_.SetDisconnectedCb([=](SessionId sess, ::network::boost_ec const& ec)
                {
                    cb(boost::any(sess), ec);
                });
    }

    bool NetTransportClient::Connect(std::string const& url)
    {

    }
    void NetTransportClient::Send(const void* data, size_t bytes, OnSndF const& cb = NULL)
    {

    }
    bool NetTransportClient::IsEstab()
    {
    }

} //namespace ucorf

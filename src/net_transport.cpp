#include "net_transport.h"
#include "logger.h"

namespace ucorf
{
    // server
    NetTransportServer::NetTransportServer()
    {
        ucorf_log_debug("NetTransportServer construct.");
    }
    NetTransportServer::~NetTransportServer()
    {
        ucorf_log_debug("NetTransportServer destruct.");
    }

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
                    ucorf_log_debug("new connection");
                    cb(boost::any(sess));
                });
    }
    void NetTransportServer::SetDisconnectedCb(OnDisconnectedF const& cb)
    {
        s_.SetDisconnectedCb([=](::network::SessionId sess, ::network::boost_ec const& ec)
                {
                    ucorf_log_debug("connection disconnect: %s", ec.message().c_str());
                    cb(boost::any(sess), ec);
                });
    }

    void NetTransportServer::SetOption(boost::any const& opt)
    {
        ::network::OptionsUser net_opt;
        if (opt.empty()) {
            net_opt.max_pack_size_ = 64 * 1024;
            return ;
        }
        net_opt = boost::any_cast<::network::OptionsUser const&>(opt);
        s_.SetSndTimeout(net_opt.sndtimeo_);
        s_.SetMaxPackSize(net_opt.max_pack_size_);
    }

    boost_ec NetTransportServer::Listen(std::string const& url)
    {
        url_ = url;
        return s_.goStart(url);
    }
    void NetTransportServer::Send(SessId id, const void* data, size_t bytes, OnSndF const& cb)
    {
        ::network::SessionId &sess = ::boost::any_cast<::network::SessionId&>(id);
        s_.GetProtocol()->Send(sess, data, bytes, cb);
    }
    std::string NetTransportServer::LocalUrl() const
    {
        return url_;
    }

    // client
    NetTransportClient::NetTransportClient()
    {
        ucorf_log_debug("NetTransportClient construct.");
    }
    NetTransportClient::~NetTransportClient()
    {
        ucorf_log_debug("NetTransportClient destruct.");
    }

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
                    ucorf_log_debug("connect sucess");
                    cb(boost::any(sess));
                });
    }
    void NetTransportClient::SetDisconnectedCb(OnDisconnectedF const& cb)
    {
        c_.SetDisconnectedCb([=](::network::SessionId sess, ::network::boost_ec const& ec)
                {
                    ucorf_log_debug("disconnect because: %s", ec.message().c_str());
                    cb(boost::any(sess), ec);
                });
    }
    void NetTransportClient::SetOption(boost::any const& opt)
    {
        ::network::OptionsUser net_opt;
        if (opt.empty()) {
            net_opt.max_pack_size_ = 64 * 1024;
            return ;
        }
        net_opt = boost::any_cast<::network::OptionsUser const&>(opt);
        c_.SetSndTimeout(net_opt.sndtimeo_);
        c_.SetMaxPackSize(net_opt.max_pack_size_);
    }

    boost_ec NetTransportClient::Connect(std::string const& url)
    {
        url_ = url;
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
    std::string NetTransportClient::RemoteUrl() const
    {
        return url_;
    }

} //namespace ucorf

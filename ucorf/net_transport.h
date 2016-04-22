#pragma once

#include "preheader.h"
#include "transport.h"
#include <libgonet/network.h>

namespace ucorf
{
    class NetTransportServer : public ITransportServer
    {
    public:
        NetTransportServer();
        ~NetTransportServer();

        virtual void Shutdown();
        virtual void SetReceiveCb(OnReceiveF const&);
        virtual void SetConnectedCb(OnConnectedF const&);
        virtual void SetDisconnectedCb(OnDisconnectedF const&);
        virtual void SetOption(boost::any const& opt);

        virtual boost_ec Listen(std::string const& url);
        virtual void Send(SessId id, const void* data, size_t bytes, OnSndF const& cb = NULL);
        virtual void Send(SessId id, std::vector<char> && buf, OnSndF const& cb = NULL);
        virtual std::string LocalUrl() const;

    private:
        ::network::Server s_;
        std::string url_;
    };

    class NetTransportClient : public ITransportClient
    {
    public:
        NetTransportClient();
        ~NetTransportClient();

        virtual void Shutdown();
        virtual void SetReceiveCb(OnReceiveF const&);
        virtual void SetConnectedCb(OnConnectedF const&);
        virtual void SetDisconnectedCb(OnDisconnectedF const&);
        virtual void SetOption(boost::any const& opt);

        virtual boost_ec Connect(std::string const& url);
        virtual void Send(const void* data, size_t bytes, OnSndF const& cb = NULL);
        virtual void Send(std::vector<char> && buf, OnSndF const& cb = NULL);
        virtual bool IsEstab();
        virtual std::string RemoteUrl() const;

    private:
        ::network::Client c_;
        std::string url_;
    };

} //namespace ucorf

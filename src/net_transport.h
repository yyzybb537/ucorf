#pragma once

#include "preheader.h"
#include "transport.h"
#include <network/network.h>

namespace ucorf
{
    class NetTransportServer : public ITransportServer
    {
    public:
        virtual void Shutdown();
        virtual void SetReceiveCb(OnReceiveF const&);
        virtual void SetConnectedCb(OnConnectedF const&);
        virtual void SetDisconnectedCb(OnDisconnectedF const&);

        virtual bool Listen(std::string const& url);
        virtual void Send(SessId id, const void* data, size_t bytes, OnSndF const& cb = NULL);

    private:
        ::network::Server s_;
    };

    class NetTransportClient : public ITransportClient
    {
    public:
        virtual void Shutdown();
        virtual void SetReceiveCb(OnReceiveF const&);
        virtual void SetConnectedCb(OnConnectedF const&);
        virtual void SetDisconnectedCb(OnDisconnectedF const&);

        virtual bool Connect(std::string const& url);
        virtual void Send(const void* data, size_t bytes, OnSndF const& cb = NULL);
        virtual bool IsEstab();

    private:
        ::network::Client c_;
    };

} //namespace ucorf

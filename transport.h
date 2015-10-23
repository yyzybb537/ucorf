#pragma once

#include "preheader.h"

namespace ucorf
{
    typedef boost::any SessId;

    class ITransport
    {
    public:
        typedef boost::function<size_t(SessId, const char* data, size_t bytes)> OnReceiveF;
        typedef boost::function<void(boost_ec const&)> OnSndF;
        typedef boost::function<void(SessId)> OnConnectedF;
        typedef boost::function<void(SessId, boost_ec const&)> OnDisconnectedF;

        virtual ~TransportBase() {}

        virtual void Shutdown() = 0;
        virtual void SetReceiveCb(OnReceiveF const&) = 0;
        virtual void SetConnectedCb(OnConnectedF const&) {}
        virtual void SetDisconnectedCb(OnDisconnectedF const&) {}
    };

    class ITransportServer : public ITransport
    {
    public:
        virtual bool Listen(std::string const& url) = 0;
        virtual void Send(SessId id, const void* data, size_t bytes, OnSndF const& cb = NULL) = 0;
    };

    class ITransportClient : public ITransport
    {
    public:
        virtual bool Connect(std::string const& url) = 0;
        virtual void Send(const void* data, size_t bytes, OnSndF const& cb = NULL) = 0;
        virtual bool IsEstab() = 0;
    };

} //namespace ucorf

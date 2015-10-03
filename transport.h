#pragma once

#include "preheader.h"

namespace ucorf
{
    struct ISessId {};
    typedef boost::shared_ptr<ISessId> SessId;

    class ITransport
    {
    public:
        typedef boost::function<void(SessId, const char* data, size_t bytes)> OnReceiveF;
        typedef boost::function<void(boost_ec const&)> OnSndF;

        virtual ~TransportBase() {}

        virtual void Shutdown() = 0;
        virtual void SetReceiveCb(OnReceiveF const&) = 0;
        virtual void Send(SessId id, const void* data, size_t bytes, OnSndF const& cb = NULL) = 0;
    };

    class ITransportServer : public ITransport
    {
    public:
        virtual bool Listen(std::string const& url) = 0;
    };

    class ITransportClient : public ITransport
    {
    public:
        virtual bool Connect(std::string const& url) = 0;
    };

} //namespace ucorf

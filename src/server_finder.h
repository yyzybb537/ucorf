#pragma once

#include "preheader.h"
#include "transport.h"

namespace ucorf
{
    class ServerFinder
    {
    public:
        typedef boost::function<void(ITransportClient*, SessId)> OnConnectedF;
        typedef boost::function<void(ITransportClient*, SessId, boost_ec const&)> OnDisconnectedF;
        typedef boost::function<ITransportClient*()> TransportFactory;

        virtual ~ServerFinder() {}

        virtual bool Init(std::string const& url, TransportFactory const& factory);

        void SetConnectedCb(OnConnectedF const& cb);
        void SetDisconnectedCb(OnDisconnectedF const& cb);

    private:
        OnConnectedF on_connect_;
        OnDisconnectedF on_disconnect_;
        TransportFactory tp_factory_;
    };

} //namespace ucorf

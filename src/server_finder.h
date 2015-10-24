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
        typedef boost::function<size_t(ITransportClient *, SessId, const char*, size_t)> OnReceiveF;
        typedef boost::function<ITransportClient*()> TransportFactory;

        virtual ~ServerFinder() {}

        virtual boost_ec Init(std::string const& url, TransportFactory const& factory);

        void SetConnectedCb(OnConnectedF const& cb);
        void SetReceiveCb(OnReceiveF const& cb);
        void SetDisconnectedCb(OnDisconnectedF const& cb);

    private:
        OnConnectedF on_connect_;
        OnReceiveF on_receive_;
        OnDisconnectedF on_disconnect_;
        TransportFactory tp_factory_;

        // single address
        boost::shared_ptr<ITransportClient> single_tp_;
    };

} //namespace ucorf

#pragma once

#include "preheader.h"
#include "transport.h"
#include "option.h"

namespace ucorf
{
    class ServerFinder
    {
    public:
        enum class eMode
        {
            single,
            zk,
        };

        typedef boost::function<void(boost::shared_ptr<ITransportClient>, SessId)> OnConnectedF;
        typedef boost::function<void(boost::shared_ptr<ITransportClient>, SessId, boost_ec const&)> OnDisconnectedF;
        typedef boost::function<size_t(boost::shared_ptr<ITransportClient>, SessId, const char*, size_t)> OnReceiveF;
        typedef boost::function<ITransportClient*()> TransportFactory;

        ServerFinder();
        virtual ~ServerFinder() {}

        virtual void Init(std::string const& url, TransportFactory const& factory);

        void SetConnectedCb(OnConnectedF const& cb);
        void SetReceiveCb(OnReceiveF const& cb);
        void SetDisconnectedCb(OnDisconnectedF const& cb);
        void SetOption(boost::shared_ptr<Option> opt);
        boost_ec ReConnect();

    private:
        OnConnectedF on_connect_;
        OnReceiveF on_receive_;
        OnDisconnectedF on_disconnect_;
        TransportFactory tp_factory_;
        bool inited_;
        std::string url_;
        eMode mode_;

        // single address
        boost::shared_ptr<ITransportClient> single_tp_;
        boost::shared_ptr<Option> opt_;
    };

} //namespace ucorf

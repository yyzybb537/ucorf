#pragma once

#include "preheader.h"
#include "transport.h"
#include "option.h"
#include "zookeeper.h"

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
        virtual ~ServerFinder();

        virtual void Init(std::string const& url, TransportFactory const& factory);

        void SetConnectedCb(OnConnectedF const& cb);
        void SetReceiveCb(OnReceiveF const& cb);
        void SetDisconnectedCb(OnDisconnectedF const& cb);
        void SetOption(boost::shared_ptr<Option> opt);
        boost_ec ReConnect();

    private:
        void OnZookeeperChilds(ZookeeperClient::Children const& nodes,
                boost::shared_ptr<bool> token, boost::shared_ptr<co_mutex> mutex);

        void OnDisconnected(boost::weak_ptr<ITransportClient> tp,
                SessId id, boost_ec const& ec, std::string url,
                boost::shared_ptr<bool> token, boost::shared_ptr<co_mutex> mutex);

        void RecursiveConnect(boost::shared_ptr<ITransportClient> sptr, std::string url,
                boost::shared_ptr<bool> token, boost::shared_ptr<co_mutex> mutex);

    private:
        OnConnectedF on_connect_;
        OnReceiveF on_receive_;
        OnDisconnectedF on_disconnect_;
        TransportFactory tp_factory_;
        bool inited_;
        std::string url_;
        eMode mode_;
        boost::shared_ptr<Option> opt_;

        // single address
        boost::shared_ptr<ITransportClient> single_tp_;

        // zookeeper address and path
        boost::shared_ptr<co_mutex> destroy_mutex_;
        boost::shared_ptr<bool> token_;
        std::string zk_addr_;
        std::string zk_path_;
        typedef std::map<std::string, boost::shared_ptr<ITransportClient>> TransportGroup;
        TransportGroup transports_;
    };

} //namespace ucorf

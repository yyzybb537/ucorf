#pragma once

#include "preheader.h"
#include "message.h"
#include "transport.h"
#include "option.h"
#include "server_finder.h"

namespace ucorf
{
    class IServiceStub;
    class ITransportClient;
    class IDispatcher;
    class ClientImpl
    {
    public:
        typedef boost::function<ITransportClient*()> TransportFactory;

        ClientImpl();
        ~ClientImpl();

        ClientImpl& SetOption(boost::shared_ptr<Option> opt);

        ClientImpl& SetUrl(std::string const& url);

        boost_ec Call(std::string const& service_name,
                std::string const& method_name,
                IMessage *request, IMessage *response);

        /// ------------------------ extend method --------------------------
    public:
        ClientImpl& SetDispatcher(std::unique_ptr<IDispatcher> && dispatcher);
        ClientImpl& SetHeaderFactory(HeaderFactory const& head_factory);
        ClientImpl& SetServerFinder(std::unique_ptr<ServerFinder> && srv_finder);
        ClientImpl& SetTransportFactory(TransportFactory const& factory);
        /// -----------------------------------------------------------------
        
    private:
        void OnConnected(boost::shared_ptr<ITransportClient> tp, SessId sess_id);
        void OnDisconnected(boost::shared_ptr<ITransportClient> tp, SessId sess_id, boost_ec const& ec);
        size_t OnReceiveData(boost::shared_ptr<ITransportClient> tp, SessId sess_id, const char* data, size_t bytes);
        void OnResponse(boost::shared_ptr<ITransportClient> tp, IHeaderPtr header, const char* data, size_t bytes);

    private:
        struct ResponseData
        {
            boost_ec ec;
            IHeaderPtr header;
            std::vector<char> data;

            ResponseData() = default;
            ResponseData(boost_ec const& e) : ec(e) {}
        };

        // 必须为2的幂
        enum { e_chan_group_count = 0x80 };

        typedef std::map<std::string, ITransportClient*> StubMap;
        typedef co_chan<ResponseData> RspChan;
        typedef std::unordered_map<std::size_t, RspChan> RspChanMap;
        struct RspChanGroup
        {
            co_mutex mtxs[e_chan_group_count];
            RspChanMap maps[e_chan_group_count];
            volatile bool closed[e_chan_group_count] = {};
        };
        typedef std::unordered_map<ITransportClient*,
                    boost::shared_ptr<RspChanGroup>> ChannelMap;
        StubMap stubs_;
        std::string url_;
        co_mutex channel_mtx_;
        ChannelMap channels_;
        std::atomic<std::size_t> msg_id_{0};
        std::atomic<std::size_t> wnd_size_{0};

        boost::shared_ptr<Option> opt_;
        std::unique_ptr<IDispatcher> dispatcher_;
        HeaderFactory head_factory_;
        std::unique_ptr<ServerFinder> default_srv_finder_;
        std::list<std::unique_ptr<ServerFinder>> srv_finders_;
        TransportFactory tp_factory_;
    };

} //namespace ucorf

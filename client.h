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
    class Client
    {
    public:
        typedef boost::function<ITransportClient*()> TransportFactory;

        Client();
        ~Client();

        Client& SetOption(Option const& opt);

        Client& SetDispatcher(IDispatcher* dispatcher);

        Client& SetHeaderFactory(HeaderFactory const& head_factory);

        Client& SetServerFinder(ServerFinder * srv_finder);

        Client& SetTransportFactory(TransportFactory const& factory);
        
        Client& SetUrl(std::string const& url);

        boost_ec Call(std::string const& service_name,
                std::string const& method_name,
                IMessage *request, IMessage *response);
        
        bool Start();

    private:
        void OnConnected(ITransportClient *tp, SessId sess_id);
        void OnDisconnected(ITransportClient *tp, SessId sess_id, boost_ec const& ec);
        size_t OnReceiveData(ITransportClient *tp, SessId sess_id, const char* data, size_t bytes);
        void OnResponse(ITransportClient *tp, IHeaderPtr header, const char* data, size_t bytes);

    private:
        struct ResponseData
        {
            boost_ec ec;
            IHeaderPtr header;
            std::vector<char> data;

            ResponseData() = default;
            ResponseData(boost_ec const& e) : ec(e) {}
        };

        typedef std::map<std::string, ITransportClient*> StubMap;
        typedef co_chan<ResponseData> RspChan;
        typedef std::unordered_map<ITransportClient*, std::unordered_map<std::size_t, RspChan>> ChannelMap;
        StubMap stubs_;
        co_mutex connect_mutex_;
        bool is_started_ = false;
        std::string url_;
        ChannelMap channels_;
        HeaderFactory head_factory_;
        TransportFactory tp_factory_;
        std::unique_ptr<ServerFinder> srv_finder_;
        std::unique_ptr<IDispatcher> dispatcher_;
        std::atomic<std::size_t> msg_id_{0};
        Option opt_;
    };

} //namespace ucorf

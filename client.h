#pragma once

#include "preheader.h"
#include "message.h"
#include "option.h"

namespace ucorf
{
    class IServiceStub;
    class ITransportClient;
    class Client
    {
    public:
        typedef boost::function<ITransportClient*()> TransportFactory;

        Client& Init(std::string const& url, TransportFactory const& factory);

        template <typename TransportType>
        Client& Init(std::string const& url)
        {
            return Init(url, []{ return static_cast<ITransportClient*>(new TransportType); });
        }

        Client& SetOption(Option const& opt);

        Client& SetDispatcher(IDispatcher* dispatcher);

        Client& SetHeaderFactory(HeaderFactory const& head_factory);

        boost_ec Call(std::string const& service_name,
                std::string const& method_name,
                IMessage *request, IMessage *response);

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
        ChannelMap channels_;
        HeaderFactory head_factory_;
        TransportFactory tp_factory_;
        IDispatcher *dispatcher_ = nullptr;
        std::atomic<std::size_t> msg_id_{0};
        Option opt_;
    };

} //namespace ucorf

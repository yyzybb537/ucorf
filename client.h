#pragma once

#include "preheader.h"
#include <map>

namespace ucorf
{
    class IServiceStub;
    class IMessage;
    class ITransportClient;
    class Client
    {
    public:
        typedef boost::function<ITransportClient*()> TransportFactory;

        Client& ConnectTo(std::string const& url, TransportFactory const& factory);

        template <typename TransportType>
        Client& ConnectTo(std::string const& url)
        {
            return ConnectTo(url, []{ return static_cast<ITransportClient*>(new TransportType); });
        }

        void SetDispatcher(IDispatcher* dispatcher);

        bool Call(std::string const& service_name,
                std::string const& method_name,
                IMessage *request, IMessage *response);

    private:
        typedef std::map<std::string, ITransportClient*> StubMap;
        StubMap stubs_;
        TransportFactory factory_;
        IDispatcher *dispatcher_;
    };

} //namespace ucorf

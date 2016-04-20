#pragma once

#include "client_impl.h"

namespace ucorf
{
    class Client
    {
    public:
        typedef boost::function<ITransportClient*()> TransportFactory;

        Client();

        Client& SetOption(boost::shared_ptr<Option> opt);

        Client& SetUrl(std::string const& url);

        boost_ec Call(std::string const& service_name,
                std::string const& method_name,
                IMessage *request, IMessage *response);

        /// ------------------------ extend method --------------------------
    public:
        Client& SetDispatcher(std::unique_ptr<IDispatcher> && dispatcher);
        Client& SetHeaderFactory(HeaderFactory const& head_factory);
        Client& SetServerFinder(std::unique_ptr<ServerFinder> && srv_finder);
        Client& SetTransportFactory(TransportFactory const& factory);
        /// -----------------------------------------------------------------
        
    private:
        boost::shared_ptr<ClientImpl> impl_;
    };

} //namespace ucorf

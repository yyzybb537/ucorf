#pragma once

#include "server_impl.h"

namespace ucorf
{
    class Server
    {
    public:
        typedef boost::function<IMessage*(std::string const&, std::string const&)> MessageFactory;

        Server();

        Server& SetOption(boost::shared_ptr<Option> opt);

        bool RegisterTo(std::string const& url);

        bool RegisterService(boost::shared_ptr<IService> service);

        void RemoveService(std::string const& service_name);

        boost_ec Listen(std::string const& url);

        /// --------------------------- extend method ---------------------------
    public:
        Server& BindTransport(std::unique_ptr<ITransportServer> && transport);
        Server& SetHeaderFactory(HeaderFactory const& head_factory);
        Server& SetRegister(boost::shared_ptr<IServerRegister> reg);
        /// ---------------------------------------------------------------------

    private:
        boost::shared_ptr<ServerImpl> impl_;
    };

} //namespace ucorf

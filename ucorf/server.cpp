#include "server.h"

namespace ucorf
{
    Server::Server()
        : impl_(new ServerImpl)
    {}

    Server& Server::SetOption(boost::shared_ptr<Option> opt)
    {
        impl_->SetOption(opt);
        return *this;
    }

    bool Server::RegisterTo(std::string const& url)
    {
        return impl_->RegisterTo(url);
    }

    bool Server::RegisterService(boost::shared_ptr<IService> service)
    {
        return impl_->RegisterService(service);
    }

    void Server::RemoveService(std::string const& service_name)
    {
        impl_->RemoveService(service_name);
    }

    boost_ec Server::Listen(std::string const& url)
    {
        return impl_->Listen(url);
    }

    Server& Server::BindTransport(std::unique_ptr<ITransportServer> && transport)
    {
        impl_->BindTransport(std::move(transport));
        return *this;
    }
    Server& Server::SetHeaderFactory(HeaderFactory const& head_factory)
    {
        impl_->SetHeaderFactory(head_factory);
        return *this;
    }
    Server& Server::SetRegister(boost::shared_ptr<IServerRegister> reg)
    {
        impl_->SetRegister(reg);
        return *this;
    }

} //namespace ucorf

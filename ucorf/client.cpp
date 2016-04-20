#include "client.h"

namespace ucorf
{
    Client::Client()
        : impl_(new ClientImpl)
    {}

    Client& Client::SetOption(boost::shared_ptr<Option> opt)
    {
        impl_->SetOption(opt);
        return *this;
    }

    Client& Client::SetUrl(std::string const& url)
    {
        impl_->SetUrl(url);
        return *this;
    }

    boost_ec Client::Call(std::string const& service_name,
            std::string const& method_name,
            IMessage *request, IMessage *response)
    {
        return impl_->Call(service_name, method_name, request, response);
    }

    /// ------------------------ extend method --------------------------
    Client& Client::SetDispatcher(std::unique_ptr<IDispatcher> && dispatcher)
    {
        impl_->SetDispatcher(std::move(dispatcher));
        return *this;
    }
    Client& Client::SetHeaderFactory(HeaderFactory const& head_factory)
    {
        impl_->SetHeaderFactory(head_factory);
        return *this;
    }
    Client& Client::SetServerFinder(std::unique_ptr<ServerFinder> && srv_finder)
    {
        impl_->SetServerFinder(std::move(srv_finder));
        return *this;
    }
    Client& Client::SetTransportFactory(TransportFactory const& factory)
    {
        impl_->SetTransportFactory(factory);
        return *this;
    }

} //namespace ucorf

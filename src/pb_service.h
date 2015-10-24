#pragma once

#include "service.h"
#include "pb_message.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

namespace ucorf
{
    using namespace ::google::protobuf;

    class Pb_Service : public IService
    {
    public:
        virtual std::string name() = 0;

        std::unique_ptr<IMessage> CallMethod(std::string const& method,
                const char *request_data, size_t request_bytes) override;

        virtual bool Call(int method_idx, Message & request, Message & response) = 0;

        virtual const ServiceDescriptor* GetDescriptor() = 0;

        const Message& GetRequestPrototype(
                const MethodDescriptor* method) const;
        const Message& GetResponsePrototype(
                const MethodDescriptor* method) const;
    };

    class Pb_ServiceStub : public IServiceStub
    {
    public:
        using IServiceStub::IServiceStub;

        boost_ec CallMethod(std::string const& method,
                Message & request, Message * response);
    };

} //namespace ucorf

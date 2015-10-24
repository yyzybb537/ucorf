#pragma once

#include "preheader.h"
#include "message.h"

namespace ucorf
{
    class IMessage;
    class IService
    {
    public:
        virtual ~IService() {}

        virtual std::string name() = 0;

        virtual IMessage* CallMethod(std::string const& method,
                const char *request_data, size_t request_bytes) = 0;
    };

    class Client;
    class IServiceStub
    {
    public:
        explicit IServiceStub(Client * c) : c_(c) {}

        virtual ~IServiceStub() {}

        virtual std::string name() = 0;

    protected:
        Client * c_;
    };

} //namespace ucorf

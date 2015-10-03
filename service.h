#pragma once

#include "preheader.h"

namespace ucorf
{
    class IMessage;
    class IService
    {
    public:
        virtual ~IService() {}

        std::string name() = 0;

        IMessage* CallMethod(std::string const& method,
                const char *request_data, size_t request_bytes) = 0;
    };

    class IServiceStub : public IService
    {

    };

} //namespace ucorf

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

        bool CallMethod(std::string const& method,
                IMessage *request, IMessage *response) = 0;
    };

    class IServiceStub : public IService
    {

    };

} //namespace ucorf

#pragma once

#include "preheader.h"

namespace ucorf
{
    enum class eDispatchAlgorithm
    {
        robin,
        random,
        con_hash,
    };

    class ITransportClient;
    class IDispatcher
    {
    public:
        typedef std::map<std::string, ITransportClient*> StubMap;

        virtual ~IDispatcher() {}

        virtual ITransportClient* Get(StubMap & stubs,
                std::string const& service_name,
                std::string const& method_name, IMessage *request);
    };

} //namespace ucorf

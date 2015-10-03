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
        virtual ~IDispatcher() {}

        virtual void Add(ITransportClient* tp);
        virtual void Del(ITransportClient* tp);

        virtual ITransportClient* Get( std::string const& service_name,
                std::string const& method_name, IMessage *request);
    };

} //namespace ucorf

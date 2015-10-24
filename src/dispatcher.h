#pragma once

#include "preheader.h"
#include "message.h"

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

        virtual void Add(boost::shared_ptr<ITransportClient> tp) = 0;
        virtual void Del(boost::shared_ptr<ITransportClient> tp) = 0;

        virtual boost::shared_ptr<ITransportClient> Get( std::string const& service_name,
                std::string const& method_name, IMessage *request) = 0;
    };

    class RobinDispatcher : public IDispatcher
    {
    public:
        virtual void Add(boost::shared_ptr<ITransportClient> tp);
        virtual void Del(boost::shared_ptr<ITransportClient> tp);

        virtual boost::shared_ptr<ITransportClient> Get( std::string const& service_name,
                std::string const& method_name, IMessage *request);

    private:
        std::vector<boost::shared_ptr<ITransportClient>> tp_list_;
        std::atomic<std::size_t> robin_idx_{0};
        co_mutex mutex_;
    };

} //namespace ucorf

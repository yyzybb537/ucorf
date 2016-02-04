#pragma once

#include "preheader.h"
#include "message.h"
#include "transport.h"
#include "conhash.h"

namespace ucorf
{
    enum class eDispatchAlgorithm
    {
        robin,
        random,
        con_hash,
    };

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
        co_rwmutex mutex_;
    };

    class HashDispatcher : public IDispatcher
    {
    public:
        typedef boost::function<std::size_t(std::string const& service_name,
                std::string const& method_name, IMessage *request)> HashF;
        typedef boost::function<std::string(std::string const& url)> HashTagF;

        virtual void Add(boost::shared_ptr<ITransportClient> tp);
        virtual void Del(boost::shared_ptr<ITransportClient> tp);

        virtual boost::shared_ptr<ITransportClient> Get(std::string const& service_name,
                std::string const& method_name, IMessage *request);

        void SetVirtualCount(std::size_t vir_count);
        void SetHashFunction(HashF fn);
        void SetHashTagFunction(HashTagF fn);

    private:
        std::string GetHashKey(boost::shared_ptr<ITransportClient> tp);

    private:
        co_rwmutex mutex_;
        std::size_t vir_count_ = 0;
        HashF hash_fn_;
        HashTagF hash_tag_fn_;
        std::size_t hash_idx_ = 0;
        con_hashtable<boost::shared_ptr<ITransportClient>> conhash_table_;
    };

} //namespace ucorf

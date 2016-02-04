#include "dispatcher.h"
#include "logger.h"

namespace ucorf
{
    // Robin
    void RobinDispatcher::Add(boost::shared_ptr<ITransportClient> tp)
    {
        std::unique_lock<co_wmutex> lock(mutex_.writer());
        tp_list_.push_back(tp);
    }
    void RobinDispatcher::Del(boost::shared_ptr<ITransportClient> tp)
    {
        std::unique_lock<co_wmutex> lock(mutex_.writer());
        auto it = std::find(tp_list_.begin(), tp_list_.end(), tp);
        if (it != tp_list_.end())
            tp_list_.erase(it);
    }

    boost::shared_ptr<ITransportClient> RobinDispatcher::Get( std::string const& service_name,
            std::string const& method_name, IMessage *request)
    {
        std::unique_lock<co_rmutex> lock(mutex_.reader());
        if (tp_list_.empty())
            return boost::shared_ptr<ITransportClient>();

        ++robin_idx_;
        robin_idx_ = robin_idx_ % tp_list_.size();
        return tp_list_[robin_idx_];
    }

    // Hash
    void HashDispatcher::Add(boost::shared_ptr<ITransportClient> tp)
    {
        std::unique_lock<co_wmutex> lock(mutex_.writer());
        std::string hashkey = GetHashKey(tp);
        if (!vir_count_)
            conhash_table_.insert(hashkey, tp);
        else
            conhash_table_.insert(hashkey, tp, vir_count_);
    }
    void HashDispatcher::Del(boost::shared_ptr<ITransportClient> tp)
    {
        std::unique_lock<co_wmutex> lock(mutex_.writer());
        std::string hashkey = GetHashKey(tp);
        conhash_table_.erase(hashkey);
    }
    boost::shared_ptr<ITransportClient> HashDispatcher::Get(
            std::string const& service_name,
            std::string const& method_name, IMessage *request)
    {
        std::unique_lock<co_rmutex> lock(mutex_.reader());
        if (hash_fn_) {
            std::size_t hashcode = hash_fn_(service_name, method_name, request);
            return conhash_table_.hget(hashcode);
        } else {
            return conhash_table_.get(std::to_string(++hash_idx_));
        }
    }
    void HashDispatcher::SetVirtualCount(std::size_t vir_count)
    {
        std::unique_lock<co_wmutex> lock(mutex_.writer());
        vir_count_ = vir_count;
    }
    void HashDispatcher::SetHashFunction(HashF fn)
    {
        std::unique_lock<co_wmutex> lock(mutex_.writer());
        hash_fn_ = fn;
    }
    void HashDispatcher::SetHashTagFunction(HashTagF fn)
    {
        std::unique_lock<co_wmutex> lock(mutex_.writer());
        hash_tag_fn_ = fn;
    }
    std::string HashDispatcher::GetHashKey(boost::shared_ptr<ITransportClient> tp)
    {
        std::string url = tp->RemoteUrl();
        if (hash_tag_fn_)
            return hash_tag_fn_(url);

        return url;
    }

} //namespace ucorf

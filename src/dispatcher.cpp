#include "dispatcher.h"

namespace ucorf
{
    void RobinDispatcher::Add(boost::shared_ptr<ITransportClient> tp)
    {
        std::unique_lock<co_mutex> lock(mutex_);
        tp_list_.push_back(tp);
    }
    void RobinDispatcher::Del(boost::shared_ptr<ITransportClient> tp)
    {
        std::unique_lock<co_mutex> lock(mutex_);
        auto it = std::find(tp_list_.begin(), tp_list_.end(), tp);
        if (it != tp_list_.end())
            tp_list_.erase(it);
    }

    boost::shared_ptr<ITransportClient> RobinDispatcher::Get( std::string const& service_name,
            std::string const& method_name, IMessage *request)
    {
        std::unique_lock<co_mutex> lock(mutex_);
        if (tp_list_.empty())
            return boost::shared_ptr<ITransportClient>();

        robin_idx_ = (++robin_idx_) % tp_list_.size();
        return tp_list_[robin_idx_];
    }

} //namespace ucorf

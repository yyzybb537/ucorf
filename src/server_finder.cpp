#include "server_finder.h"
#include <boost/algorithm/string.hpp>
#include <boost/weak_ptr.hpp>
#include "error.h"
#include "logger.h"

namespace ucorf
{
    ServerFinder::ServerFinder()
        : opt_(new Option), destroy_mutex_(new co_mutex), token_(new bool(true))
    {}

    ServerFinder::~ServerFinder()
    {
        std::unique_lock<co_mutex> lock(*destroy_mutex_);
        *token_ = false;
        if (!zk_addr_.empty()) {
            auto zk = ZookeeperClientMgr::getInstance().GetZookeeperClient(zk_addr_);
            zk->Unwatch(zk_path_, this);
        }
    }

    void ServerFinder::Init(std::string const& url, TransportFactory const& factory)
    {
        tp_factory_ = factory;
        url_ = url;

        if (boost::istarts_with(url_, "zk://")) {
            auto addr_path = ZookeeperClientMgr::getInstance().ParseZookeeperUrl(url_);
            if (addr_path.first.empty()) {
                ucorf_log_error("zookeeper(%s) url parse error!", url_.c_str());
                return ;
            }

            auto zk = ZookeeperClientMgr::getInstance().GetZookeeperClient(addr_path.first);
            zk->Watch(addr_path.second, boost::bind(&ServerFinder::OnZookeeperChilds, this, _1, token_, destroy_mutex_), this);
            mode_ = eMode::zk;
            return ;
        }

        // single connection.
        mode_ = eMode::single;
        single_tp_.reset(tp_factory_());
        if (!opt_->transport_opt.empty())
            single_tp_->SetOption(opt_->transport_opt);
        boost::weak_ptr<ITransportClient> weak(single_tp_);
        single_tp_->SetConnectedCb([=](SessId id){ on_connect_(weak.lock(), id); });
        single_tp_->SetReceiveCb([=](SessId id, const char* data, size_t len){ return on_receive_(weak.lock(), id, data, len); });
        single_tp_->SetDisconnectedCb(boost::bind(&ServerFinder::OnDisconnected, this, weak, _1, _2, url, token_, destroy_mutex_));
        go [=]{ ReConnect(); };
    }

    void ServerFinder::SetConnectedCb(OnConnectedF const& cb)
    {
        on_connect_ = cb;
    }

    void ServerFinder::SetReceiveCb(OnReceiveF const& cb)
    {
        on_receive_ = cb;
    }

    void ServerFinder::SetDisconnectedCb(OnDisconnectedF const& cb)
    {
        on_disconnect_ = cb;
    }

    void ServerFinder::SetOption(boost::shared_ptr<Option> opt)
    {
        opt_ = opt;
    }

    boost_ec ServerFinder::ReConnect()
    {
        if (mode_ == eMode::zk) {
            return MakeUcorfErrorCode(eUcorfErrorCode::ec_no_estab);
        } else if (mode_ == eMode::single) {
            if (!single_tp_->IsEstab())
                return single_tp_->Connect(url_);

            return boost_ec();
        }

        return MakeUcorfErrorCode(eUcorfErrorCode::ec_unsupport_protocol);
    }

    void ServerFinder::OnZookeeperChilds(ZookeeperClient::Children const& nodes,
            boost::shared_ptr<bool> token, boost::shared_ptr<co_mutex> mutex)
    {
        std::unique_lock<co_mutex> lock(*mutex, std::defer_lock);
        if (!lock.try_lock()) return ;
        if (!*token) return ;

        TransportGroup tp_group;
        for (auto &node : nodes)
        {
            ucorf_log_debug(" zookeeper node: %s/%s", zk_path_.c_str(), node.c_str());
            std::string url = ZookeeperClientMgr::getInstance().ZookeeperNode2Url(node);
            if (transports_.count(url)) {
                tp_group[url].swap(transports_[url]);
            } else {
                boost::shared_ptr<ITransportClient> tp(tp_factory_());
                tp_group[url] = tp;
                if (!opt_->transport_opt.empty())
                    tp->SetOption(opt_->transport_opt);
                boost::weak_ptr<ITransportClient> weak(tp);
                tp->SetConnectedCb([=](SessId id){ on_connect_(weak.lock(), id); });
                tp->SetReceiveCb([=](SessId id, const char* data, size_t len){ return on_receive_(weak.lock(), id, data, len); });
                tp->SetDisconnectedCb(boost::bind(&ServerFinder::OnDisconnected, this, weak, _1, _2, url, token_, destroy_mutex_));
                go [=]{ RecursiveConnect(tp, url, token_, destroy_mutex_); };
            }
        }

        tp_group.swap(transports_);

        for (auto &kv : tp_group) {
            auto &tp = kv.second;
            if (!tp) continue;

            ucorf_log_warn("Node %s was miss on zookeeper.", tp->RemoteUrl().c_str());
            tp->Shutdown();
        }
    }

    void ServerFinder::OnDisconnected(boost::weak_ptr<ITransportClient> weak,
            SessId id, boost_ec const& ec, std::string url,
            boost::shared_ptr<bool> token, boost::shared_ptr<co_mutex> mutex)
    {
        std::unique_lock<co_mutex> lock(*mutex, std::defer_lock);
        if (!lock.try_lock()) return ;
        if (!*token) return ;

        auto sptr = weak.lock();
        if (!sptr) return ;

        if (mode_ == eMode::single || transports_.count(url))
            go [=]{ RecursiveConnect(sptr, url, token, mutex); };

        if (on_disconnect_)
            on_disconnect_(sptr, id, ec);
    }

    void ServerFinder::RecursiveConnect(boost::shared_ptr<ITransportClient> sptr, std::string url,
            boost::shared_ptr<bool> token, boost::shared_ptr<co_mutex> mutex)
    {
        boost_ec ec = sptr->Connect(url);
        if (!ec) {
            ucorf_log_debug("connect to %s success.", url.c_str());
        } else {
            ucorf_log_error("connect to %s error: %s.", url.c_str(), ec.message().c_str());
            co_sleep(3000);

            if (sptr->IsEstab()) return ;

            {
                std::unique_lock<co_mutex> lock(*mutex, std::defer_lock);
                if (!lock.try_lock()) return ;
                if (!*token) return ;
                if (mode_ == eMode::zk && !transports_.count(url)) return ;
            }

            go [=]{ RecursiveConnect(sptr, url, token, mutex); };
        }
    }

} //namespace ucorf

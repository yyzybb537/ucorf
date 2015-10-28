#include "server_finder.h"
#include <boost/algorithm/string.hpp>
#include <boost/weak_ptr.hpp>
#include "error.h"
#include "logger.h"

namespace ucorf
{
    ServerFinder::ServerFinder()
        : opt_(new Option), token_(new bool(true))
    {}

    ServerFinder::~ServerFinder()
    {
        std::unique_lock<co_mutex> lock(destroy_mutex_);
        *token_ = false;
        auto zk = ZookeeperClientMgr::getInstance().GetZookeeperClient(zk_addr_);
        zk->Unwatch(zk_path_, this);
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
            zk->Watch(addr_path.second, boost::bind(&ServerFinder::OnZookeeperChilds, this, _1, token_), this);
            mode_ = eMode::zk;
            return ;
        }

        // single connection.
        mode_ = eMode::single;
        single_tp_.reset(factory());
        if (!opt_->transport_opt.empty())
            single_tp_->SetOption(opt_->transport_opt);
        boost::weak_ptr<ITransportClient> weak(single_tp_);
        single_tp_->SetConnectedCb([=](SessId id){ on_connect_(weak.lock(), id); });
        single_tp_->SetReceiveCb([=](SessId id, const char* data, size_t len){ return on_receive_(weak.lock(), id, data, len); });
        single_tp_->SetDisconnectedCb([=](SessId id, boost_ec const& ec){ on_disconnect_(weak.lock(), id, ec); });
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
            boost::shared_ptr<bool> token)
    {
        std::unique_lock<co_mutex> lock(destroy_mutex_, std::defer_lock);
        if (!lock.try_lock()) return ;
        if (!*token) return ;

        // TODO
    }

} //namespace ucorf

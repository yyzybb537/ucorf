#include "server_finder.h"
#include <boost/algorithm/string.hpp>
#include <boost/weak_ptr.hpp>
#include "error.h"

namespace ucorf
{
    ServerFinder::ServerFinder()
        : opt_(new Option)
    {}

    boost_ec ServerFinder::Init(std::string const& url, TransportFactory const& factory)
    {
        tp_factory_ = factory;
        url_ = url;

        if (boost::istarts_with(url_, "zk://")) {
            // TODO: support zookeeper.
            mode_ = eMode::zk;
            return MakeUcorfErrorCode(eUcorfErrorCode::ec_unsupport_protocol);
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
        return single_tp_->Connect(url_);
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
            // TODO: support zookeeper.
            return MakeUcorfErrorCode(eUcorfErrorCode::ec_unsupport_protocol);
        } else if (mode_ == eMode::single) {
            if (!single_tp_->IsEstab())
                return single_tp_->Connect(url_);

            return boost_ec();
        }

        return MakeUcorfErrorCode(eUcorfErrorCode::ec_unsupport_protocol);
    }

} //namespace ucorf

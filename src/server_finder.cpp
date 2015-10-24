#include "server_finder.h"
#include <boost/algorithm/string.hpp>
#include "error.h"

namespace ucorf
{
    boost_ec ServerFinder::Init(std::string const& url, TransportFactory const& factory)
    {
        tp_factory_ = factory;

        if (boost::istarts_with(url, "zk://")) {
            // TODO: support zookeeper.
            return MakeUcorfErrorCode(eUcorfErrorCode::ec_unsupport_protocol);
        }

        // single connection.
        single_tp_.reset(factory());
        single_tp_->SetConnectedCb([=](SessId id){ on_connect_(single_tp_.get(), id); });
        single_tp_->SetReceiveCb([=](SessId id, const char* data, size_t len){ return on_receive_(single_tp_.get(), id, data, len); });
        single_tp_->SetDisconnectedCb([=](SessId id, boost_ec const& ec){ on_disconnect_(single_tp_.get(), id, ec); });
        return single_tp_->Connect(url);
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

} //namespace ucorf

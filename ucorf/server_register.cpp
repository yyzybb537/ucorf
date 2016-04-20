#include "server_register.h"
#include <boost/algorithm/string.hpp>
#include "logger.h"
#include "zookeeper.h"
#include <boost/regex.hpp>

namespace ucorf
{
    bool ZookeeperRegister::Register(std::string url, std::string srv_info)
    {
        auto addr_path = ZookeeperClientMgr::getInstance().ParseZookeeperUrl(url);
        if (addr_path.first.empty()) return false;

        std::string addr = addr_path.first;
        std::string path = addr_path.second;
        std::string zk_node = ZookeeperClientMgr::getInstance().Url2ZookeeperNode(srv_info);
        if (zk_node.empty()) {
            ucorf_log_error("convert srv_info(%s) to zookeeper node error", srv_info.c_str());
            return false;
        }
        path += "/" + zk_node;

        auto zk = ZookeeperClientMgr::getInstance().GetZookeeperClient(addr);
        if (!zk->DelayCreateEphemeralNode(path)) {
            ucorf_log_error("create node(%s) to zookeeper(%s) error.", path.c_str(), addr.c_str());
            return false;
        }

        destinations_.insert(std::make_pair(addr, path));
        return true;
    }

    void ZookeeperRegister::Unregister()
    {
        for (auto &kv : destinations_) {
            auto zk = ZookeeperClientMgr::getInstance().GetZookeeperClient(kv.first);
            zk->DeleteNode(kv.second);
        }
    }

} //namespace ucorf

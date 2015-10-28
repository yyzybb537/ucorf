#include "server_register.h"
#include <boost/algorithm/string.hpp>
#include "logger.h"
#include "zookeeper.h"
#include <boost/regex.hpp>

namespace ucorf
{
    bool ZookeeperRegister::Register(std::string url, std::string srv_info)
    {
        const char* zk_prefix = "zk://";
        if (!boost::istarts_with(url, zk_prefix)) {
            ucorf_log_error("zookeeper url must be starts with \"zk://\"");
            return false;
        }

        size_t addr_begin = strlen(zk_prefix);
        size_t path_begin = url.find('/', addr_begin);
        if (std::string::npos == path_begin) {
            ucorf_log_error("zookeeper url must be have path, starts with '/'");
            return false;
        }

        std::string addr = url.substr(addr_begin, path_begin - addr_begin);
        std::string path = url.substr(path_begin, -1);
        std::string zk_node = Url2ZookeeperNode(srv_info);
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

    std::string ZookeeperRegister::Url2ZookeeperNode(std::string url)
    {
        static ::boost::regex re("((.*)://)?([^/]+)");
        boost::smatch result;
        bool ok = boost::regex_match(url, result, re);
        if (!ok) {
            return "";
        }

        std::string proto = result[2].str();
        if (proto.empty()) proto = "TCP";
        std::string addr = result[3];
        return addr + ":" + proto;
    }

} //namespace ucorf

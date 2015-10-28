#include "zookeeper.h"
#include "logger.h"

namespace ucorf
{
    void ZookeeperClient::__watcher_fn(zhandle_t *zh, int type, 
        int state, const char *path, void *watcherCtx)
    {
        ZookeeperClient *self = (ZookeeperClient*)watcherCtx;
        self->OnWatch(zh, type, state, path);
    }

    void ZookeeperClient::Init(std::string zk_host)
    {
        host_ = zk_host;
        go [this]{ this->Connect(); };
    }

    bool ZookeeperClient::Watch(std::string path, WatchCb const& cb, void* key)
    {
        std::unique_lock<co_mutex> lock(mutex_);
        WatcherMap &m = zk_watchers_[path];
        bool ok = m.insert(WatcherMap::value_type(key, cb)).second;
        if (!ok)
            return false;

        if (!zk_) return true;

        if (__Watch(path, cb)) {
            m.erase(key);
            return false;
        }

        return true;
    }

    bool ZookeeperClient::__Watch(std::string path, WatchCb const& cb)
    {
        if (!zk_) {
            ucorf_log_error("watch zookeeper node(%s) error "
                    " because zhandle_t is null.", path.c_str());
            return false;
        }

        // get children.
        String_vector children;
retry:
        int ret = zoo_get_children(zk_, path.c_str(), 1, &children);
        switch (ret) {
            case ZOK:
            {
                Children c;
                for (int32_t i = 0; i < children.count; ++i)
                    c.push_back(children.data[i]);
                deallocate_String_vector(&children);
                if (cb)
                    cb(c);
                else {
                    auto &m = zk_watchers_[path];
                    for (auto &kv : m)
                        kv.second(c);
                }
                return true;
            }

            case ZNONODE:
            {
                // Create path recursive.
                if (!CreateNode(path, eCreateNodeFlags::normal, true, false)) {
                    return false;
                }

                goto retry;
            }

            default:
            {
                ucorf_log_error("zoo_get_children from zookeeper node(%s) error: %s",
                        path.c_str(), zerror(errno));
                return false;
            }
        }
    }

    void ZookeeperClient::Unwatch(std::string path, void* key)
    {
        std::unique_lock<co_mutex> lock(mutex_);
        auto it = zk_watchers_.find(path);
        if (it != zk_watchers_.end())
            it->second.erase(key);
    }

    bool ZookeeperClient::CreateNode(std::string path, eCreateNodeFlags flags,
            bool recursive, bool is_lock)
    {
        std::unique_lock<co_mutex> lock(mutex_, std::defer_lock);
        if (is_lock) lock.lock();

        if (!zk_) {
            ucorf_log_error("create zookeeper node(%s) error "
                    " because zhandle_t is null.", path.c_str());
            return false;
        }

        ACL_vector acl = {};
        char buf[1] = {};
        if (recursive) {
            std::string::size_type pos = 1;
            do {
                pos = path.find("/", pos);
                if (pos == std::string::npos) break;
                std::string parent = path.substr(0, pos);
                int ret = zoo_create(zk_, parent.c_str(), nullptr, -1, &acl, 0, buf, 0);
                if (ret != ZOK && ret != ZNODEEXISTS) {
                    ucorf_log_error("create zookeeper node(%s) returns %d error: %s",
                            parent.c_str(), ret, zerror(errno));
                    return false;
                }
            } while (pos != std::string::npos);
        }

        int cflag = 0;
        if (flags == eCreateNodeFlags::ephemeral)
            cflag = ZOO_EPHEMERAL;
        else if (flags == eCreateNodeFlags::sequence)
            cflag = ZOO_SEQUENCE;

        int ret = zoo_create(zk_, path.c_str(), nullptr, -1, &acl, cflag, buf, 0);
        if (ret != ZOK && ret != ZNODEEXISTS) {
            ucorf_log_error("create zookeeper node(%s) returns %d error: %s",
                    path.c_str(), ret, zerror(errno));
            return false;
        }

        return true;
    }

    bool ZookeeperClient::DeleteNode(std::string path)
    {
        std::unique_lock<co_mutex> lock(mutex_);
        if (!zk_) {
            ucorf_log_error("delete zookeeper node(%s) error "
                    " because zhandle_t is null.", path.c_str());
            return false;
        }

        int ret = zoo_delete(zk_, path.c_str(), -1);
        if (ret != ZOK || ret != ZNONODE) {
            ucorf_log_error("delete zookeeper node(%s) returns %d error: %s",
                    path.c_str(), ret, zerror(errno));
            return false;
        }

        return true;
    }

    void ZookeeperClient::Connect()
    {
        zhandle_t *zk = zookeeper_init(host_.c_str(), &ZookeeperClient::__watcher_fn,
                ZookeeperClientMgr::getInstance().GetTimeout(),
                nullptr, this, 0);
        if (!zk) {
            ucorf_log_error("connect to zookeeper (%s) error: %s. delay 3s retry...",
                    host_.c_str(), zerror(errno));
            go [this]{ 
                co_sleep(3000);
                this->Connect(); 
            };
            return ;
        }
    }

    void ZookeeperClient::OnWatch(zhandle_t *zh, int type, int state, const char *path)
    {
        std::unique_lock<co_mutex> lock(mutex_);
        ucorf_log_notice("zookeeper trigger type=%d state=%d path=%s ",
                type, state, path ? path : "(nil)");

        if (type == ZOO_SESSION_EVENT) {
            if (state == ZOO_CONNECTED_STATE) {
                zk_ = zh;
                for (auto &kv : zk_watchers_)
                    __Watch(kv.first);
            } else {
                if (zk_) {
                    zookeeper_close(zk_);
                    zk_ = nullptr;
                }

                go [this]{ this->Connect(); };
                return ;
            }
        } else if (type == ZOO_CHILD_EVENT) {
            __Watch(path);
        } else {
            // ignore other event.
        }
    }

    ZookeeperClientMgr& ZookeeperClientMgr::getInstance()
    {
        static ZookeeperClientMgr obj;
        return obj;
    }

    void ZookeeperClientMgr::SetTimeout(int timeo)
    {
        zk_timeout_ = timeo;
    }

    int ZookeeperClientMgr::GetTimeout()
    {
        return zk_timeout_;
    }

    boost::shared_ptr<ZookeeperClient> ZookeeperClientMgr::GetZookeeperClient(std::string zk_host)
    {
        std::unique_lock<co_mutex> lock(mutex_);
        auto it = zk_clients_.find(zk_host);
        if (it == zk_clients_.end()) {
            auto c = boost::make_shared<ZookeeperClient>();
            c->Init(zk_host);
            zk_clients_[zk_host] = c;
            return c;
        }

        return it->second;
    }

} //namespace ucorf

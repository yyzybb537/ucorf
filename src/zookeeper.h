#pragma once

#include "preheader.h"
#include "option.h"
#include <zookeeper/zookeeper.h>
#include <boost/noncopyable.hpp>

namespace ucorf
{
    class ZookeeperClient : public boost::noncopyable
    {
    public:
        enum class eCreateNodeFlags
        {
            normal,
            ephemeral,
            sequence,
        };

        typedef std::vector<std::string> Children;
        typedef boost::function<void(Children const&)> WatchCb;
        typedef std::map<void*, WatchCb> WatcherMap;
        typedef std::map<std::string, WatcherMap> ZkWatchers;
        typedef boost::function<void()> OnWatchF;

        ZookeeperClient();

        void Init(std::string zk_host);

        bool WaitForConnected(unsigned timeout_ms);

        bool Watch(std::string path, WatchCb const& cb, void* key);

        void Unwatch(std::string path, void* key);

        bool CreateNode(std::string path, eCreateNodeFlags flags,
                bool recursive = true, bool is_lock = true);

        bool DelayCreateEphemeralNode(std::string path, bool is_lock = true);

        bool DeleteNode(std::string path);

    private:
        void Connect();
        void OnWatch(zhandle_t *zh, int type, int state, std::string path);
        bool __Watch(std::string path, WatchCb const& cb = NULL);
        static void __watcher_fn(zhandle_t *zh, int type, 
                int state, const char *path, void *watcherCtx);

    private:
        zhandle_t *zk_ = nullptr;
        std::string host_;
        ZkWatchers zk_watchers_;
        std::set<std::string> ephemeral_nodes_;
        co_chan<OnWatchF> event_chan_;
        co_mutex mutex_;
    };

    class ZookeeperClientMgr : public boost::noncopyable
    {
    public:
        static ZookeeperClientMgr& getInstance();

        void SetTimeout(int timeo);
        int GetTimeout();

        boost::shared_ptr<ZookeeperClient> GetZookeeperClient(std::string zk_host);

    private:
        std::map<std::string, boost::shared_ptr<ZookeeperClient> > zk_clients_;
        int zk_timeout_ = 10000;
        co_mutex mutex_;
    };

} //namespace ucorf

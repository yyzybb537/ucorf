#pragma once

#include "preheader.h"

namespace ucorf
{
    class IServerRegister
    {
    public:
        virtual ~IServerRegister() {}

        virtual bool Register(std::string url, std::string srv_info) = 0;

        virtual void Unregister() = 0;
    };

    class ZookeeperRegister : public IServerRegister
    {
    public:
        virtual bool Register(std::string url, std::string srv_info);

        virtual void Unregister();

        std::string Url2ZookeeperNode(std::string url);

    private:
        std::multimap<std::string, std::string> destinations_;
    };

} //namespace ucorf

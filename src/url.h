#pragma once

#include "preheader.h"

namespace ucorf
{
    class Url
    {
    public:
        enum class eProto
        {
            tcp,
            tls,
            udp,
            http,
            https,
            zookeeper,
        };

        eProto proto_;
        std::string ori_address_;
        std::vector<std::pair<std::string, uint16_t>> address_;
        std::string path_;

        explicit Url(std::string const& str);
    };

} //namespace ucorf

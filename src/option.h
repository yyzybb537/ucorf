#pragma once

#include "preheader.h"

namespace ucorf
{
    struct Option
    {
        std::size_t request_wnd_size = -1;
        int rcv_timeout_ms = 10000;

        boost::any transport_opt;
    };

} //namespace ucorf

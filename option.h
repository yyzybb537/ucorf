#pragma once

#include "preheader.h"

namespace ucorf
{
    struct Option
    {
        std::size_t request_wnd_size = -1;
        int snd_timeout_ms = 0;
        int rcv_timeout_ms = 0;
    };

} //namespace ucorf

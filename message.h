#pragma once

#include "preheader.h"

namespace ucorf
{
    class IMessage
    {
    public:
        virtual ~IMessage() {}

        virtual bool Serialize(void* buf, std::size_t len) = 0;
        virtual std::size_t ByteSize() = 0;
        virtual std::size_t Parse(const void* buf, std::size_t len) = 0;
    };

} //namespace ucorf

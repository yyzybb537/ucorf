#pragma once

#include "preheader.h"
#include "message.h"

namespace ucorf
{
    class Pb_Message : public IMessage
    {
    public:
        virtual bool Serialize(void* buf, std::size_t len);
        virtual std::size_t ByteSize();
        virtual std::size_t Parse(const void* buf, std::size_t len);
    };
} //namespace ucorf

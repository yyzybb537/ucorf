#pragma once

#include "preheader.h"
#include "message.h"
#include <memory>

namespace ucorf
{
    class Hprose_Message : public IMessage
    {
    public:
        Hprose_Message() = default;
        explicit Hprose_Message(std::string const& body);

        virtual bool Serialize(void* buf, std::size_t len);
        virtual std::size_t ByteSize();
        virtual std::size_t Parse(const void* buf, std::size_t len);

        std::string body_;
    };
} //namespace ucorf

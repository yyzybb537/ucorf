#pragma once

#include "message.h"

namespace ucorf
{
    struct Hprose_Head : public IHeader
    {
    public:
        virtual void SetId(std::size_t id);
        virtual void SetFollowBytes(std::size_t bytes);

        virtual void SetType(eHeaderType) {}
        virtual void SetService(std::string const&) {}
        virtual void SetMethod(std::string const&) {}

        virtual std::size_t GetId();
        virtual eHeaderType GetType();
        virtual std::size_t GetFollowBytes();
        virtual std::string GetService() { return ""; }
        virtual std::string GetMethod() { return ""; }

        virtual bool Serialize(void* buf, std::size_t len);
        virtual std::size_t ByteSize();
        virtual std::size_t Parse(const void* buf, std::size_t len);

        static IHeaderPtr Factory();

        uint32_t callid;
        uint32_t body_length;
    };
} //namespace ucorf

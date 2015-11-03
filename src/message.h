#pragma once

#include "preheader.h"

namespace ucorf
{
    enum class eHeaderType
    {
        request,
        oneway_request,
        response,
    };

    class IHeader
    {
    public:
        virtual ~IHeader() {}

        virtual void SetId(std::size_t id) = 0;
        virtual void SetType(eHeaderType type) = 0;
        virtual void SetFollowBytes(std::size_t bytes) = 0;
        virtual void SetService(std::string const& srv) = 0;
        virtual void SetMethod(std::string const& method) = 0;

        virtual std::size_t GetId() = 0;
        virtual eHeaderType GetType() = 0;
        virtual std::size_t GetFollowBytes() = 0;
        virtual std::string const& GetService() = 0;
        virtual std::string const& GetMethod() = 0;

        virtual bool Serialize(void* buf, std::size_t len) = 0;
        virtual std::size_t ByteSize() = 0;
        virtual std::size_t Parse(const void* buf, std::size_t len) = 0;
    };
    typedef boost::shared_ptr<IHeader> IHeaderPtr;
    typedef boost::function<IHeaderPtr()> HeaderFactory;

    class IMessage
    {
    public:
        virtual ~IMessage() {}

        virtual bool Serialize(void* buf, std::size_t len) = 0;
        virtual std::size_t ByteSize() = 0;
        virtual std::size_t Parse(const void* buf, std::size_t len) = 0;
    };

    struct UcorfHead : public IHeader
    {
    public:
        virtual void SetId(std::size_t id);
        virtual void SetType(eHeaderType type);
        virtual void SetFollowBytes(std::size_t bytes);
        virtual void SetService(std::string const& srv);
        virtual void SetMethod(std::string const& mthd);

        virtual std::size_t GetId();
        virtual eHeaderType GetType();
        virtual std::size_t GetFollowBytes();
        virtual std::string const& GetService();
        virtual std::string const& GetMethod();

        virtual bool Serialize(void* buf, std::size_t len);
        virtual std::size_t ByteSize();
        virtual std::size_t Parse(const void* buf, std::size_t len);

        static IHeaderPtr Factory();

        static const unsigned char magic_code = 0xf8;
        uint8_t calltype;
        uint32_t callid;
        uint32_t body_length;
        std::string service;
        std::string method;
    };

} //namespace ucorf

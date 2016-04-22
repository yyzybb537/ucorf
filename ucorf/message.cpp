#include "message.h"

namespace ucorf
{
    void UcorfHead::SetId(std::size_t id)
    {
        callid = id;
    }
    void UcorfHead::SetType(eHeaderType type)
    {
        calltype = (uint8_t)type;
    }
    void UcorfHead::SetFollowBytes(std::size_t bytes)
    {
        body_length = bytes;
    }
    void UcorfHead::SetService(std::string const& srv)
    {
        service = srv;
    }
    void UcorfHead::SetMethod(std::string const& mthd)
    {
        method = mthd;
    }

    std::size_t UcorfHead::GetId()
    {
        return callid;
    }
    eHeaderType UcorfHead::GetType()
    {
        return (eHeaderType)calltype;
    }
    std::size_t UcorfHead::GetFollowBytes()
    {
        return body_length;
    }
    std::string UcorfHead::GetService()
    {
        return service;
    }
    std::string UcorfHead::GetMethod()
    {
        return method;
    }

    bool UcorfHead::Serialize(void* buf, std::size_t len)
    {
        if (len < ByteSize()) return false;
        *(unsigned char*)buf = magic_code;
        *(uint8_t*)((char*)buf + 1) = calltype;
        *(uint32_t*)((char*)buf + 2) = htonl(callid);
        *(uint32_t*)((char*)buf + 6) = htonl(body_length);
        *(uint16_t*)((char*)buf + 10) = htons(service.size());
        *(uint16_t*)((char*)buf + 12) = htons(method.size());
        memcpy((char*)buf + 14, service.data(), service.size());
        memcpy((char*)buf + 14 + service.size(), method.data(), method.size());
        return true;
    }
    std::size_t UcorfHead::ByteSize()
    {
        return sizeof(unsigned char) + sizeof(calltype) +
            sizeof(callid) + sizeof(body_length) +
            4 + service.size() + method.size();
    }
    std::size_t UcorfHead::Parse(const void* buf, std::size_t len)
    {
        if (len < 14) return 0;
        if (*(unsigned char*)buf != magic_code) return 0;
        uint16_t service_len = htons(*(uint16_t*)((char*)buf + 10));
        uint16_t method_len = htons(*(uint16_t*)((char*)buf + 12));
        if ((uint16_t)len < 14 + service_len + method_len) return 0;

        calltype = *(uint8_t*)((char*)buf + 1);
        callid = htonl(*(uint32_t*)((char*)buf + 2));
        body_length = htonl(*(uint32_t*)((char*)buf + 6));
        service.assign((char*)buf + 14, service_len);
        method.assign((char*)buf + 14 + service_len, method_len);
        return 14 + service_len + method_len;
    }

    IHeaderPtr UcorfHead::Factory()
    {
        return boost::static_pointer_cast<IHeader>(boost::make_shared<UcorfHead>());
    }

} //namespace ucorf

#include "hprose_header.h"

namespace ucorf {

void Hprose_Head::SetId(std::size_t id)
{
    callid = id;
}
void Hprose_Head::SetFollowBytes(std::size_t bytes)
{
    body_length = bytes;
}
std::size_t Hprose_Head::GetId()
{
    return callid;
}
eHeaderType Hprose_Head::GetType()
{
    return eHeaderType::request;
}
std::size_t Hprose_Head::GetFollowBytes()
{
    return body_length;
}
bool Hprose_Head::Serialize(void * buf, std::size_t len)
{
    if (len < 8) return false;
    *(uint32_t*)((char*)buf) = htonl(body_length);
    *(uint32_t*)((char*)buf + 4) = htonl(callid);
    return true;
}
std::size_t Hprose_Head::Parse(const void* buf, std::size_t len)
{
    if (len < 8) return false;
    body_length = htonl(*(uint32_t*)((char*)buf));
    callid = htonl(*(uint32_t*)((char*)buf + 4));
    return 8;
}
std::size_t Hprose_Head::ByteSize()
{
    return 8;
}
IHeaderPtr Hprose_Head::Factory()
{
    return boost::static_pointer_cast<IHeader>(boost::make_shared<Hprose_Head>());
}

} //namespace ucorf

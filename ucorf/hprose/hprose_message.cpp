#include "hprose_message.h"

namespace ucorf {

Hprose_Message::Hprose_Message(std::string const& body)
    : body_(body)
{
}
bool Hprose_Message::Serialize(void * buf, std::size_t len)
{
    if (len < ByteSize()) return false;
    memcpy(buf, body_.data(), len);
    return true;
}
std::size_t Hprose_Message::ByteSize()
{
    return body_.size();
}
std::size_t Hprose_Message::Parse(const void * buf, std::size_t len)
{
    body_.assign((const char*)buf, len);
    return len;
}

} //namespace ucorf

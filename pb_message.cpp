#include "pb_message.h"

namespace ucorf
{
    Pb_Message::Pb_Message(::google::protobuf::Message* msg)
        : msg_(msg)
    {
    }

    bool Pb_Message::Serialize(void* buf, std::size_t len)
    {
        if (!msg_) return false;
        return msg_->SerializeToArray(buf, len);
    }
    std::size_t Pb_Message::ByteSize()
    {
        if (!msg_) return 0;
        return msg_->ByteSize();
    }
    std::size_t Pb_Message::Parse(const void* buf, std::size_t len)
    {
        if (!msg_) return 0;
        return msg_->ParseFromArray(buf, len);
    }

} //namespace ucorf

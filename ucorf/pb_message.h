#pragma once

#include "preheader.h"
#include "message.h"
#include <memory>
#include <google/protobuf/message.h>

namespace ucorf
{
    class Pb_Message : public IMessage
    {
    public:
        Pb_Message() = default;
        Pb_Message(::google::protobuf::Message* msg, bool own = true);
        Pb_Message(std::unique_ptr<::google::protobuf::Message> && msg);
        ~Pb_Message();

        virtual bool Serialize(void* buf, std::size_t len);
        virtual std::size_t ByteSize();
        virtual std::size_t Parse(const void* buf, std::size_t len);
        
        ::google::protobuf::Message* msg_ = nullptr;
        bool own_ = false;
    };
} //namespace ucorf

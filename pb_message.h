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
        Pb_Message(::google::protobuf::Message* msg);

        virtual bool Serialize(void* buf, std::size_t len);
        virtual std::size_t ByteSize();
        virtual std::size_t Parse(const void* buf, std::size_t len);
        
        std::unique_ptr<::google::protobuf::Message> msg_;
    };
} //namespace ucorf

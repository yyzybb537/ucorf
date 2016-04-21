#include "hprose_service.h"
#include <sstream>

namespace ucorf {
namespace hprose {

std::unique_ptr<IMessage> Hprose_Service::CallMethod(std::string const&,
        const char *request_data, size_t request_bytes)
{
    Buffer reader(request_data, request_bytes);

    char flag = reader.get();
    if (flag == hprose::TagEnd) {
        // return function list.
        return std::unique_ptr<IMessage>(new Hprose_Message(GetFunctionList()));
    }

    if (flag == hprose::TagCall) {
        std::string method;
        if (!reader.Read(method))
            return std::unique_ptr<IMessage>(new Hprose_Message("Es10\"Error Args\"z"));
        return std::unique_ptr<IMessage>(new Hprose_Message(Call(method, reader)));
    }

    Buffer writer;
    writer.Write(hprose::TagError);
    writer.Write("No support protocol tag.");
    writer.Write(hprose::TagEnd);
    return std::unique_ptr<IMessage>(new Hprose_Message(writer.str()));
}

std::string Hprose_Service::GetFunctionList()
{
    Buffer writer;
    writer.Write(hprose::TagFunctions);
    std::vector<std::string> funcs;
    funcs.reserve(functions_.size());
    for (auto &kv : functions_)
        funcs.push_back(kv.first);
    writer.Write(funcs);
    writer.Write(hprose::TagEnd);
    return writer.str();
}

std::string Hprose_Service::Call(std::string const& method, Buffer & reader)
{
    boost::shared_ptr<CalleeBase> sptr;

    {
        std::unique_lock<co_mutex> lock(func_mutex_);
        auto it = functions_.find(method);
        if (functions_.end() == it)
            it = functions_.find("*");

        if (functions_.end() != it)
            sptr = it->second;
    }

    if (!sptr) {
        // no callee, returns error.
        Buffer writer;
        writer.Write(hprose::TagError);
        writer.Write(std::string("No Callee ") + method);
        writer.Write(hprose::TagEnd);
        return writer.str();
    }

    return sptr->Call(reader);
}

} //namespace hprose
} //namespace ucorf

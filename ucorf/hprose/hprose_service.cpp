#include "hprose_service.h"
#include <sstream>

namespace ucorf {

std::unique_ptr<IMessage> Hprose_Service::CallMethod(std::string const&,
        const char *request_data, size_t request_bytes)
{
    std::stringstream ss;
    ss.rdbuf()->sputn(request_data, request_bytes);
    hprose::HproseReader reader(ss);

    char flag = ss.get();
    if (flag == hprose::HproseTags::TagEnd) {
        // return function list.
        return std::unique_ptr<IMessage>(new Hprose_Message(GetFunctionList()));
    }

    if (flag == hprose::HproseTags::TagCall) {
        std::string method = reader.ReadString();
        return std::unique_ptr<IMessage>(new Hprose_Message(Call(method, reader)));
    }

    std::stringstream ws;
    hprose::HproseWriter writer(ws);
    ws << hprose::HproseTags::TagError;
    writer.WriteString("No support protocol tag.");
    ws << hprose::HproseTags::TagEnd;
    return std::unique_ptr<IMessage>(new Hprose_Message(ws.str()));
}

std::string Hprose_Service::GetFunctionList()
{
    std::stringstream ss;
    hprose::HproseWriter writer(ss);

    ss << hprose::HproseTags::TagFunctions;
    std::vector<std::string> funcs;
    funcs.reserve(functions_.size());
    for (auto &kv : functions_)
        funcs.push_back(kv.first);
    writer.WriteList(funcs);
    ss << hprose::HproseTags::TagEnd;
    return ss.str();
}

std::string Hprose_Service::Call(std::string const& method,
        hprose::HproseReader & reader)
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
        std::stringstream ss;
        hprose::HproseWriter writer(ss);
        ss << hprose::HproseTags::TagError;
        writer.WriteString(std::string("No Callee ") + method);
        ss << hprose::HproseTags::TagEnd;
        return ss.str();
    }

    return sptr->Call(reader);
}

} //namespace ucorf

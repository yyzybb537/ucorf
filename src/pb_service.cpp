#include "pb_service.h"
#include "client.h"

namespace ucorf
{
    std::unique_ptr<IMessage> Pb_Service::CallMethod(std::string const& method,
            const char *request_data, size_t request_bytes)
    {
        const MethodDescriptor* method_descriptor =
            GetDescriptor()->FindMethodByName(method);

        if (!method_descriptor) return nullptr;

        std::unique_ptr<Message> request(GetRequestPrototype(method_descriptor).New());
        if (!request->ParseFromArray(request_data, request_bytes))
            return nullptr;

        std::unique_ptr<Message> response(GetResponsePrototype(method_descriptor).New());

        bool ok = Call(method_descriptor->index(), *request, *response);
        if (!ok) return nullptr;

        std::unique_ptr<IMessage> rsp_msg(new Pb_Message(std::move(response)));
        return std::move(rsp_msg);
    }

    const Message& Pb_Service::GetRequestPrototype(
            const MethodDescriptor* method) const
    {
        return *MessageFactory::generated_factory()->GetPrototype(method->input_type());
    }
    const Message& Pb_Service::GetResponsePrototype(
            const MethodDescriptor* method) const
    {
        return *MessageFactory::generated_factory()->GetPrototype(method->output_type());
    }

    boost_ec Pb_ServiceStub::CallMethod(std::string const& method,
            Message & request, Message * response)
    {
        Pb_Message req(&request, false);
        Pb_Message rsp(response, false);
        return c_->Call(name(), method, &req, response ? &rsp : (Pb_Message*)nullptr);
    }

} //namespace ucorf

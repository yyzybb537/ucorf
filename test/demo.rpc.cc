#include "demo.rpc.h"

namespace Echo
{
    std::string UcorfEchoService::name()
    {
        return "EchoService";
    }

    bool UcorfEchoService::Call(int method_idx, Message & request, Message & response)
    {
        switch (method_idx) {
            case 0:
                return Echo((EchoRequest &)request, (EchoResponse &)response);
            default:
                return false;
        }
    }

    const ServiceDescriptor* UcorfEchoService::GetDescriptor()
    {
        return EchoService::GetDescriptor();
    }

    UcorfEchoServiceStub::UcorfEchoServiceStub(Client * c)
        : Pb_ServiceStub(c) {}

    std::string UcorfEchoServiceStub::name()
    {
        return "EchoService";
    }

    std::shared_ptr<EchoResponse> UcorfEchoServiceStub::Echo(EchoRequest & request, boost_ec * ec)
    {
        std::shared_ptr<EchoResponse> response = std::make_shared<EchoResponse>();
        boost_ec e = CallMethod("Echo", request, response.get());
        if (ec) {
            if (e) *ec = e;
            else ec->clear();
        }

        return response;
    }

    boost_ec UcorfEchoServiceStub::Echo(EchoRequest & request, EchoResponse * response)
    {
        return CallMethod("Echo", request, response);
    }

}

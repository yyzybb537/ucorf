#include <ucorf/pb_service.h>
#include "echo.pb.h"

namespace Echo
{
    using namespace ::google::protobuf;
    using namespace ::ucorf;

    class UcorfEchoService : public Pb_Service, private EchoService
    {
    public:
        virtual std::string name();

        virtual bool Call(int method_idx, Message & request, Message & response);

        virtual const ServiceDescriptor* GetDescriptor();

        virtual bool Echo(EchoRequest & request, EchoResponse & response) = 0;
    };

    class UcorfEchoServiceStub : public Pb_ServiceStub
    {
    public:
        explicit UcorfEchoServiceStub(Client * c);

        virtual std::string name();

        virtual std::shared_ptr<EchoResponse> Echo(EchoRequest & request, boost_ec * ec = nullptr);

        virtual boost_ec Echo(EchoRequest & request, EchoResponse * response);
    };
}

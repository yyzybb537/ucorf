#include <ucorf/client.h>
#include <ucorf/net_transport.h>
#include <ucorf/dispatcher.h>
#include <ucorf/server_finder.h>
#include "demo.rpc.h"
#include <iostream>
#include <boost/smart_ptr/make_shared.hpp>
using std::cout;
using std::endl;
using namespace Echo;

int main(int argc, char **argv)
{
    using namespace ucorf;

    const char* url = "tcp://127.0.0.1:8080";
    if (argc > 1)
        url = argv[1];

    auto header_factory = [] {
        return boost::static_pointer_cast<IHeader>(boost::make_shared<UcorfHead>());
    };

    auto tp_factory = [] {
        return (ITransportClient*)new NetTransportClient;
    };

    std::unique_ptr<RobinDispatcher> dispatcher(new RobinDispatcher);
    std::unique_ptr<ServerFinder> finder(new ServerFinder);
    Client client;
    client.SetTransportFactory(tp_factory)
        .SetHeaderFactory(header_factory)
        .SetServerFinder(std::move(finder))
        .SetDispatcher(std::move(dispatcher))
        .SetUrl(url);
    UcorfEchoServiceStub stub(&client);

//    go [&]{
//retry:
//        EchoRequest request;
//        request.set_code(1);
//        EchoResponse response;
//
//        boost_ec ec = stub.Echo(request, &response);
//        if (ec) {
//            cout << "rpc call error: " << ec.message() << endl;
//        } else {
//            cout << "rpc call success, response.code=" << response.code() << endl;
//        }
//
//        co_sleep(1000);
//        goto retry;
//    };

    go [&]{
retry:
        EchoRequest request;
        request.set_code(1);

        boost_ec ec;
        std::shared_ptr<EchoResponse> rsp = stub.Echo(request, &ec);
        if (ec) {
            cout << "rpc call error: " << ec.message() << endl;
        } else {
            cout << "rpc call success, response.code=" << rsp->code() << endl;
        }

        co_sleep(1000);
        goto retry;
    };

    co_sched.RunLoop();
    return 0;
}

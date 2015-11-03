#include <ucorf/client.h>
#include <ucorf/net_transport.h>
#include <ucorf/dispatcher.h>
#include <ucorf/server_finder.h>
#include "echo.rpc.h"
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

    Client client;
    client.SetUrl(url);
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

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

    const char* url = "zk://127.0.0.1:2181/ucorf/test";
    if (argc > 1)
        url = argv[1];

    std::unique_ptr<HashDispatcher> dispatcher(new HashDispatcher);
    Client client;
    client.SetDispatcher(std::move(dispatcher)).SetUrl(url);
    UcorfEchoServiceStub stub(&client);

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

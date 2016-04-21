#include <ucorf/client.h>
#include <ucorf/net_transport.h>
#include <ucorf/dispatcher.h>
#include <ucorf/server_finder.h>
#include <ucorf/hprose/hprose_service.h>
#include "echo.rpc.h"
#include <iostream>
#include <boost/smart_ptr/make_shared.hpp>
using std::cout;
using std::endl;

int main(int argc, char **argv)
{
    using namespace ucorf;

    const char* url = "tcp://127.0.0.1:48080";
    if (argc > 1)
        url = argv[1];

    Client client;
    client.SetUrl(url);
    client.SetHeaderFactory(&Hprose_Head::Factory);
    Hprose_ServiceStub stub(&client);

    go [&]{
retry:
        boost_ec ec;
        int rsp = stub.CallMethod<int>("add", ec, 1);
        if (ec) {
            cout << "rpc call error: " << ec.message() << endl;
        } else {
            cout << "rpc call success, response=" << rsp << endl;
        }

        co_sleep(1000);
        goto retry;
    };

    co_sched.RunLoop();
    return 0;
}

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

static std::atomic<size_t> g_count{0};
static std::atomic<size_t> g_error{0};

void show_status()
{
    static size_t last_count = 0;
    static size_t last_error = 0;
    size_t ok = g_count - last_count;
    size_t err = g_error - last_error;
    last_count = g_count;
    last_error = g_error;
    cout << "ok: " << ok << endl;
    cout << "error: " << err << endl;
}

int main(int argc, char** argv)
{
    using namespace ucorf;

    int concurrecy = 100;
    if (argc > 1) {
        concurrecy = atoi(argv[1]);
    }

    FILE * lg = fopen("log", "a+");
    if (!lg) {
        cout << "open log error:" << strerror(errno) << endl;
        return 1;
    }

    co_sched.GetOptions().debug_output = lg;
    co_sched.GetOptions().debug = co::dbg_all;

    auto header_factory = [] {
        return boost::static_pointer_cast<IHeader>(boost::make_shared<UcorfHead>());
    };

    auto tp_factory = [] {
        return (ITransportClient*)new NetTransportClient;
    };

    std::unique_ptr<RobinDispatcher> dispatcher(new RobinDispatcher);
    std::unique_ptr<ServerFinder> finder(new ServerFinder);
    Client client;
    client.SetTransportFactory(tp_factory).SetHeaderFactory(header_factory)
        .SetServerFinder(std::move(finder)).SetDispatcher(std::move(dispatcher)).SetUrl("tcp://127.0.0.1:8080");
    UcorfEchoServiceStub stub(&client);

    for (int i = 0; i < concurrecy; ++i)
        go [&]{
            for (;;) {
                EchoRequest request;
                request.set_code(1);
                EchoResponse response;

                boost_ec ec = stub.Echo(request, &response);
                if (ec) {
                    co_yield;
                    ++g_error;
                    cout << "rpc call error: " << ec.message() << endl;
                } else {
                    ++g_count;
                }
            }
        };

    go []{
        for (;;) {
            show_status();
            co_sleep(1000);
        }
    };

    co_sched.RunLoop();
    return 0;
}

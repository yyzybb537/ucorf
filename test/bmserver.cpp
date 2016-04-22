#include <ucorf/server.h>
#include <ucorf/net_transport.h>
#include "echo.rpc.h"
#include <iostream>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/thread.hpp>
using std::cout;
using std::endl;
using namespace Echo;

static std::atomic<size_t> g_count{0};

struct MyEcho : public ::Echo::UcorfEchoService
{
    virtual bool Echo(EchoRequest & request, EchoResponse & response)
    {
        ++g_count;
        response.set_code(request.code());
        return true;
    }
};

void show_status()
{
    static size_t last_count = 0;
    size_t qps = g_count - last_count;
    last_count = g_count;
    cout << "qps: " << qps << endl;
}

int main(int argc, char **argv)
{
    using namespace ucorf;

    if (argc > 1 && std::string(argv[1]) == "-h") {
        printf("Usage: bmclient.t [ThreadCount] [Address]\n");
        return 0;
    }

    int thread_c = 1;
    if (argc > 1)
        thread_c = atoi(argv[1]);

    std::string url = "tcp://127.0.0.1:48080";
    if (argc > 2)
        url = argv[2];

    ::network::OptionsUser tp_opt;
    tp_opt.max_pack_size_ = 40960;
    auto opt = boost::make_shared<Option>();
    opt->transport_opt = tp_opt;

    Server server;
    server.SetOption(opt).RegisterService(boost::shared_ptr<IService>(new MyEcho));
    boost_ec ec = server.Listen(url);
    if (ec) {
        cout << "listen error: " << ec.message() << endl;
        return 1;
    } else {
        cout << "start success, listen " << url << ", thread=" << thread_c << endl;
    }

    go []{
        for (;;) {
            show_status();
            co_sleep(1000);
        }
    };

    boost::thread_group tg;
    for (int i = 0; i < thread_c; ++i)
        tg.create_thread([]{ co_sched.RunLoop(); });
    tg.join_all();
    return 0;
}

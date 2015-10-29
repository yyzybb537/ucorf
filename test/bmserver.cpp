#include <ucorf/server.h>
#include <ucorf/net_transport.h>
#include "demo.rpc.h"
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

    auto header_factory = [] {
        return boost::static_pointer_cast<IHeader>(boost::make_shared<UcorfHead>());
    };

    int thread_c = 2;
    if (argc > 1)
        thread_c = atoi(argv[1]);

    std::string url = "tcp://127.0.0.1:8080";
    if (argc > 2)
        url = argv[2];

    std::unique_ptr<NetTransportServer> tp(new NetTransportServer);
    boost_ec ec = tp->Listen(url);
    if (ec) {
        cout << "listen error: " << ec.message() << endl;
        return 1;
    } else {
        cout << "start success, listen " << url << ", thread=" << thread_c << endl;
    }

    ::network::OptionsUser tp_opt;
    tp_opt.max_pack_size_ = 40960;
    auto opt = boost::make_shared<Option>();
    opt->transport_opt = tp_opt;

    std::shared_ptr<IService> echo_srv(new MyEcho);
    Server server;
    server.SetOption(opt)
        .BindTransport(std::move(tp))
        .SetHeaderFactory(header_factory)
        .RegisterService(echo_srv);

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

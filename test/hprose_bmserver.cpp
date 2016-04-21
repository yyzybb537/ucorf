#include <ucorf/server.h>
#include <ucorf/net_transport.h>
#include <ucorf/hprose/hprose_service.h>
#include <iostream>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/thread.hpp>
using std::cout;
using std::endl;
using namespace ucorf;
using namespace ucorf::hprose;

static std::atomic<size_t> g_count{0};

int add(int v)
{
//    cout << "call add, value=" << v << endl;
    return v + v;
}

void show_status()
{
    static size_t last_count = 0;
    size_t qps = g_count - last_count;
    last_count = g_count;
    cout << "qps: " << qps << endl;
}

int main(int argc, char **argv)
{

    if (argc > 1 && std::string(argv[1]) == "-h") {
        printf("Usage: bmclient.t [ThreadCount] [Address]\n");
        return 0;
    }

    int thread_c = 4;
    if (argc > 1)
        thread_c = atoi(argv[1]);

    std::string url = "tcp://127.0.0.1:48080";
    if (argc > 2)
        url = argv[2];

    ::network::OptionsUser tp_opt;
    tp_opt.max_pack_size_ = 40960;
    auto opt = boost::make_shared<Option>();
    opt->transport_opt = tp_opt;

    Hprose_Service *hp_srv = new Hprose_Service;
    hp_srv->RegisterFunction("add", boost::function<int(int)>(&add));
    boost::shared_ptr<IService> srv(hp_srv);
    Server server;
    server.SetOption(opt);
    server.SetHeaderFactory(&Hprose_Head::Factory);
    server.RegisterService(srv);

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

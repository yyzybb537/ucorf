#include <ucorf/server.h>
#include <ucorf/net_transport.h>
#include "demo.rpc.h"
#include <iostream>
#include <boost/smart_ptr/make_shared.hpp>
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

int main()
{
    using namespace ucorf;

    auto header_factory = [] {
        return boost::static_pointer_cast<IHeader>(boost::make_shared<UcorfHead>());
    };

    std::unique_ptr<NetTransportServer> tp(new NetTransportServer);
    boost_ec ec = tp->Listen("tcp://192.168.1.106:8080");
    if (ec) {
        cout << "listen error: " << ec.message() << endl;
        return 1;
    } else {
        cout << "start success" << endl;
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

    co_sched.RunLoop();
    return 0;
}

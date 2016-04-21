#include <ucorf/server.h>
#include <ucorf/net_transport.h>
#include <ucorf/hprose/hprose_service.h>
#include <iostream>
#include <boost/smart_ptr/make_shared.hpp>
using std::cout;
using std::endl;

int add(int v)
{
    cout << "call add, value=" << v << endl;
    return v + v;
}

int main(int argc, char **argv)
{
    using namespace ucorf;
    using namespace ucorf::hprose;

    const char* url = "tcp://127.0.0.1:48080";
    if (argc > 1)
        url = argv[1];

    Hprose_Service *hp_srv = new Hprose_Service;
    hp_srv->RegisterFunction("add", boost::function<int(int)>(&add));
    boost::shared_ptr<IService> srv(hp_srv);
    Server server;
    server.SetHeaderFactory(&Hprose_Head::Factory);
    server.RegisterService(srv);
    boost_ec ec = server.Listen(url);
    if (ec) {
        cout << "listen " << url << " error: " << ec.message() << endl;
        return 1;
    } else {
        cout << "start success: " << url << endl;
    }

    co_sched.RunLoop();
    return 0;
}

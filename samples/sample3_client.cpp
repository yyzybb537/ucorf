/********************************
 * Sample2: 从Zookeeper获取服务端地址的Client
*********************************/
#include <ucorf/ucorf.h>
#include "echo.rpc.h"
#include <iostream>
using namespace Echo;

int main(int argc, char **argv)
{
    // 1.创建Client对象
    ucorf::Client client;

    // 2.设置服务端注册的ZK地址和节点, 格式如下：
    client.SetUrl("zk://127.0.0.1:2181/ucorf/test");

    // 3.创建Stub对象，并绑定Client
    UcorfEchoServiceStub stub(&client);

    // 4.启动一个协程
    go [&]{
        co_sleep(500);

        // 5.构造RPC调用参数
        EchoRequest request;
        request.set_code(1);

        // 6.调用RPC接口
        std::shared_ptr<EchoResponse> rsp = stub.Echo(request);

        // 7.获取返回值
        std::cout << "Response: " << rsp->code() << endl;
    };

    // 8.启动协程框架主循环
    co_sched.RunLoop();
    return 0;
}


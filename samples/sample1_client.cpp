/********************************
 * Sample1: 最简单的Client
*********************************/
#include <ucorf/ucorf.h>
#include "echo.rpc.h"
#include <iostream>
using namespace Echo;

int main(int argc, char **argv)
{
    // 1.创建Client对象
    ucorf::Client client;

    // 2.设置服务端地址
    client.SetUrl("tcp://127.0.0.1:8080");

    // 3.创建Stub对象，并绑定Client
    UcorfEchoServiceStub stub(&client);

    // 4.启动一个协程
    go [&]{
        // 为了分布式部署时不存在依赖性，client在后台自动尝试连接服务端、断线重连
        // 所以写demo测试时, 要等待客户端连上以后再调用
        // 实际在服务中由于时用户触发的行为，所以不必如此
        // 和很多框架一样，client的连接尝试是在主循环中开始的, 因此要在协程中sleep
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


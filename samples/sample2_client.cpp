/********************************
 * Sample2: 对一个服务端建立多条连接的客户端
*********************************/
#include <ucorf/ucorf.h>
#include "echo.rpc.h"
#include <iostream>
using namespace Echo;
using namespace ucorf;

int main(int argc, char **argv)
{
    // 1.创建Client对象
    ucorf::Client client;

    // 2.创建多条连接
    for (int i = 0; i < 10; ++i)
        client.SetServerFinder(std::unique_ptr<ServerFinder>(new ServerFinder));

    // 3.设置服务端地址
    client.SetUrl("tcp://127.0.0.1:8080");

    // 4.创建Stub对象，并绑定Client
    UcorfEchoServiceStub stub(&client);

    /* 5.启动多个协程 */ 
    for (int i = 0; i < 10; ++i)
        go [&]{
            co_sleep(1000);

            // 6.构造RPC调用参数
            EchoRequest request;
            request.set_code(1);

            // 7.调用RPC接口
            boost_ec ec;
            std::shared_ptr<EchoResponse> rsp = stub.Echo(request, &ec);

            if (ec) {
                // 调用失败
                std::cout << "call error:" << ec.message() << std::endl;
            } else {
                // 8.获取返回值
                std::cout << "Response: " << rsp->code() << endl;
            }
        };

    // 6.修改Log输出目的地
    ucorf::Logger::default_instance().Reopen("/dev/null");

    // 7.启动协程框架主循环
    co_sched.RunLoop();
    return 0;
}


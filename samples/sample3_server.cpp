/********************************
 * Sample3: 注册到zookeeper的Server
*********************************/
#include <ucorf/ucorf.h>
#include "echo.rpc.h"
#include <iostream>
using namespace Echo;
using namespace ucorf;

// 1.继承代码生成器生成的类，并重写定义的rpc接口
struct MyEcho : public ::Echo::UcorfEchoService
{
    virtual bool Echo(EchoRequest & request, EchoResponse & response)
    {
        // 处理请求
        response.set_code(request.code());
        return true;
    }
};

int main()
{
    // 2.创建server对象
    ucorf::Server server;

    // 3.注册MyEcho服务
    server.RegisterService(boost::shared_ptr<IService>(new MyEcho));

    // 4.监听端口
    server.Listen("tcp://127.0.0.1:8080");     // 监听本机8080端口
    server.Listen("udp://127.0.0.1:8080");     // 也可以监听UDP端口

    // 5.注册到Zookeeper
    server.RegisterTo("zk://127.0.0.1:2181/ucorf/test");
    server.RegisterTo("zk://127.0.0.1:2181/ucorf/test2"); // 可以同时注册到不同ZK的多个节点上

    // 6.启动协程框架主循环
    co_sched.RunLoop();     

    return 0;
}


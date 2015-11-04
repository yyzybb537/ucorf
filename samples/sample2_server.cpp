/********************************
 * Sample2: 同时监听多个端口的Server
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

    // 4.监听2个端口
    server.Listen("tcp://127.0.0.1:8080");     // 监听本机8080端口
    server.Listen("tcp://127.0.0.1:8081");     // 监听本机8081端口

    // 5.启动协程框架主循环
    co_sched.RunLoop();     

    return 0;
}


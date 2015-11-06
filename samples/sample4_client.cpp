/********************************
 * Sample2: 使用一致性HASH算法负载均衡的Client
*********************************/
#include <ucorf/ucorf.h>
#include "echo.rpc.h"
#include <iostream>
using namespace Echo;

int main(int argc, char **argv)
{
    // 1.创建Client对象
    ucorf::Client client;

    // 2.设置一致性HASH的dispatcher
    ucorf::HashDispatcher *dispatcher(new ucorf::HashDispatcher);

    /// (可选)设置Hash函数 
    dispatcher->SetHashFunction([](std::string const& service_name,
                std::string const& method_name, IMessage *request) -> std::size_t
            {
                // 根据code做hash, 可以把同样code的请求发到同一个server
                // 注意：整数不要直接使用std::hash处理，得到的hash值不够离散。
                EchoRequest *req = (EchoRequest*)request;
                return std::hash<std::string>()(std::to_string(req->code()));
            });
    client.SetDispatcher(std::unique_ptr<ucorf::IDispatcher>(dispatcher));

    // 3.设置服务端注册的ZK地址和节点, 格式如下：
    client.SetUrl("zk://127.0.0.1:2181/ucorf/test");

    // 4.创建Stub对象，并绑定Client
    UcorfEchoServiceStub stub(&client);

    // 5.启动协程
    go [&]{
        co_sleep(500);

        // 多次调用, 根据一致性HASH算法分配到ZK节点下的各个服务器
        for (int i = 0; i < 10; ++i)
        {
            // 6.构造RPC调用参数
            EchoRequest request;
            request.set_code(i);

            // 7.调用RPC接口
            std::shared_ptr<EchoResponse> rsp = stub.Echo(request);

            // 8.获取返回值
            std::cout << "Response: " << rsp->code() << endl;
        }
    };

    // 9.启动协程框架主循环
    co_sched.RunLoop();
    return 0;
}


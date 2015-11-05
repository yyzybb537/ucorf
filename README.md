# Ucorf概述
使用C++11编写的通用协程RPC框架，尤其适合做微服务架构的服务底层框架。
在服务发现、通信协议包头、通信协议包体、底层通信通道、负载均衡算法等维度上均可扩展。
####服务发现：
    默认支持IP直连和Zookeeper服务发现
####通信协议：
    默认使用protobuf，提供protoc插件，自动生成相关代码
####底层通信通道：
    默认支持tcp、udp协议
####负载均衡：
    默认提供robin、一致性hash两种算法

## Linux上编译与安装
  ucorf使用CMake作为构建工具, 在git代码库中执行以下命令:
  
    $ git submodule update --init
    $ mkdir build
    $ cd build
    $ cmake .. -DCMAKE_BUILD_TYPE=RELEASE
    $ sudo make install
 
 至此, 安装完成. 如果要编译debug模式的库，将
 
    $ cmake .. -DCMAKE_BUILD_TYPE=RELEASE
 命令换成
 
    $ cmake ..
    
## 安装依赖的库
### DEBIAN系Linux：

    $ sudo apt-get install libzookeeper-mt-dev -y
    $ sudo apt-get install libprotobuf-dev -y
    $ sudo apt-get install python-protobuf -y
    $ sudo apt-get install protobuf-compiler -y
    $ sudo apt-get install libboost-all-dev -y

## 基础用法
#### 一.URL
  ucorf使用url来表示地址信息, 格式如下:
  
    <protocol>://<ip>:<port>
  例如：
  
    tcp://127.0.0.1:8080
    udp://127.0.0.1:8081
  目前仅支持tcp和udp两种协议。
  
#### 二.协程
  ucorf是基于协程框架实现的，关于协程的好处及相关知识参见: https://github.com/yyzybb537/cpp_features
  
#### 三.协议定义与代码生成
  ucorf默认使用protobuf作为通信协议.(如果不满足需求, 请自行扩展)
  
  protobuf的IDL语法在此不再赘述, 一个简单例子在test/echo.proto中。
  
  生成代码命令如下：
  
        $ protoc --cpp_out=. echo.proto
        $ protoc --rpc_out=. echo.proto
        
  第一行命令是protobuf自带的生成CPP代码的命令，会生成echo.pb.h和echo.pb.cc两个文件。
  
  第二行命令是使用ucorf扩展插件的命令，会生成echo.rpc.h和echo.rpc.cc两个文件。
  
  使用时只需#include "echo.rpc.h"即可。
  
#### 四.示例
  一个简单的server:
  ~~~~~~~~~~cpp
#include <ucorf/ucorf.h>
#include "echo.rpc.h"
#include <iostream>
using namespace Echo;

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

    // 4.启动监听端口
    ucorf::boost_ec ec = server.Listen("tcp://127.0.0.1:8080");     // 监听本机8080端口

    // 5.启动协程框架主循环
    co_sched.RunLoop();     

    return 0;
}
  ~~~~~~~~~~
  一个简单的client:
  ~~~~~~~~~~cpp
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
  ~~~~~~~~~~
  
  

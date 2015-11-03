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
 
    $cmake ..

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


# ucorf
使用C++11编写的通用协程RPC框架.

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


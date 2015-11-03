cmake_minimum_required(VERSION 2.8)
project(test)
set(CMAKE_CXX_FLAGS "-std=c++11 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -O3")
include_directories("${PROJECT_SOURCE_DIR}/../test")
set(PROTO_LIST ${PROJECT_SOURCE_DIR}/../test/echo.pb.cc ${PROJECT_SOURCE_DIR}/../test/echo.rpc.cc)
set(LINK_ARGS "-lucorf -lnetwork -lcoroutine -lprotobuf -lboost_thread -lboost_system -lboost_regex -lzookeeper_mt -ldl -lpthread")

add_executable(sample1_server.t sample1_server.cpp ${PROTO_LIST})
target_link_libraries(sample1_server.t ${LINK_ARGS})

add_executable(sample1_client.t sample1_client.cpp ${PROTO_LIST})
target_link_libraries(sample1_client.t ${LINK_ARGS})

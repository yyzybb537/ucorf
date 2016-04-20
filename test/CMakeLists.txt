cmake_minimum_required(VERSION 2.8)

project(test)
if (CMAKE_BUILD_TYPE)
else()
    set(CMAKE_BUILD_TYPE RELEASE)
endif()

message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

include_directories("${PROJECT_SOURCE_DIR}/..")
include_directories("${PROJECT_SOURCE_DIR}/../ucorf")
include_directories("${PROJECT_SOURCE_DIR}/../ucorf/hprose")
link_directories("${PROJECT_SOURCE_DIR}/../build")
link_directories("${PROJECT_SOURCE_DIR}/../build/third_party/libgonet")
link_directories("${PROJECT_SOURCE_DIR}/../build/third_party/libgonet/third_party/libgo")

set(CMAKE_CXX_FLAGS "-std=c++11 -g")
set(LINK_ARGS "-lucorf -llibgonet -llibgo -lprotobuf -lboost_thread -lboost_system -lboost_coroutine -lboost_context -lboost_regex -lboost_thread -lzookeeper_mt -ldl -lpthread -static -static-libgcc -static-libstdc++")

aux_source_directory("${PROJECT_SOURCE_DIR}" SRC_LIST)

foreach(var ${SRC_LIST})
    string(FIND "${var}" ".cpp" is_cpp)
    if (${is_cpp} EQUAL -1)
    else()
        string(REGEX REPLACE ".*/" "" var ${var})
        string(REGEX REPLACE ".cpp" "" tgt ${var})

        string(FIND "${var}" "hprose" is_hprose)
        if (${is_hprose} EQUAL -1)
            add_executable(${tgt}.t ${var} echo.rpc.cc echo.pb.cc)
        else()
            add_executable(${tgt}.t ${var})
        endif()
        target_link_libraries(${tgt}.t ${LINK_ARGS})
    endif()
endforeach(var)

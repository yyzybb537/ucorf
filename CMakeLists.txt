cmake_minimum_required(VERSION 2.8)

project(ucorf)

if (CMAKE_BUILD_TYPE)
else()
    set(CMAKE_BUILD_TYPE RELEASE)
endif()

message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Werror")
set(CMAKE_CXX_FLAGS_PROFILE "-g -pg -O3 ${CMAKE_CXX_FLAGS}")

include_directories("${PROJECT_SOURCE_DIR}/ucorf")
include_directories("${PROJECT_SOURCE_DIR}/ucorf/hprose")
include_directories("${PROJECT_SOURCE_DIR}/third_party/libgonet")
include_directories("${PROJECT_SOURCE_DIR}/third_party/libgonet/third_party/libgo")
include_directories("${PROJECT_SOURCE_DIR}/third_party/libgonet/third_party/libgo/libgo")
include_directories("${PROJECT_SOURCE_DIR}/third_party/libgonet/third_party/libgo/libgo/linux")

aux_source_directory(${PROJECT_SOURCE_DIR}/ucorf SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/ucorf/hprose SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/ucorf/hprose/io SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/ucorf/hprose/common SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/ucorf/hprose/ext SRC_LIST)
#message("source list:\n${SRC_LIST}")
add_library(ucorf ${SRC_LIST})

set(CMAKE_INSTALL_PREFIX "/usr/local")
install(DIRECTORY ${PROJECT_SOURCE_DIR}/ucorf/ DESTINATION "include/ucorf" FILES_MATCHING PATTERN *.h PATTERN *.hpp)
install(FILES ${PROJECT_BINARY_DIR}/libucorf.a DESTINATION "lib")
install(PROGRAMS ${PROJECT_SOURCE_DIR}/proto_gen/protoc-gen-rpc DESTINATION "bin")
install(PROGRAMS ${PROJECT_SOURCE_DIR}/proto_gen/descriptor_pb2.py DESTINATION "bin")
install(PROGRAMS ${PROJECT_SOURCE_DIR}/proto_gen/plugin_pb2.py DESTINATION "bin")

if (NOT NO_SUB)
    add_subdirectory("${PROJECT_SOURCE_DIR}/third_party/libgonet")
endif()

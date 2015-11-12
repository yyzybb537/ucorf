cmake_minimum_required(VERSION 2.8)

project(ucorf)
set(CXX_FLAGS "-std=c++11 -Wall -Werror")
set(CMAKE_CXX_FLAGS "-g ${CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ${CMAKE_CXX_FLAGS}")

include_directories("${PROJECT_SOURCE_DIR}/third_party/cpp_features")
include_directories("${PROJECT_SOURCE_DIR}/third_party/cpp_features/coroutine")
include_directories("${PROJECT_SOURCE_DIR}/third_party/cpp_features/coroutine/linux")

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)
add_library(ucorf ${SRC_LIST})

set(CMAKE_INSTALL_PREFIX "/usr/local")
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/ DESTINATION "include/ucorf" FILES_MATCHING PATTERN *.h)
install(FILES ${PROJECT_BINARY_DIR}/libucorf.a DESTINATION "lib")
install(PROGRAMS ${PROJECT_SOURCE_DIR}/proto_gen/protoc-gen-rpc DESTINATION "bin")
install(PROGRAMS ${PROJECT_SOURCE_DIR}/proto_gen/descriptor_pb2.py DESTINATION "bin")
install(PROGRAMS ${PROJECT_SOURCE_DIR}/proto_gen/plugin_pb2.py DESTINATION "bin")

add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/cpp_features)

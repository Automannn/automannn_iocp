cmake_minimum_required(VERSION 3.15)
project(automannn_iocp)

#include_directories(server)
#include_directories(client)

add_subdirectory(server)
add_subdirectory(client)

#设置编译完成后so库生成的路径为 app/libs（可根据自己的习惯设置）
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/)
set(CMAKE_CXX_STANDARD 11)

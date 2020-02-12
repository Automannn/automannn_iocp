cmake_minimum_required(VERSION 3.15)
project(automannn_iocp_client)

#设置编译完成后so库生成的路径为 app/libs（可根据自己的习惯设置）
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/)
set(CMAKE_CXX_STANDARD 11)

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_executable(automannn_iocp_client ${SRC_LIST} ${HEADERS} main.cpp main.h)
target_link_libraries(automannn_iocp_client -lws2_32)
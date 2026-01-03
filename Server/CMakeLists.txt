cmake_minimum_required(VERSION 3.10)
project(LoginServerDemo)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-g -O0")

# 包含头文件路径
include_directories(${PROJECT_SOURCE_DIR})

# 添加可执行文件
add_executable(login_server
    server_main.cpp
    CommandSocket.cpp
    LoginServer.cpp
    ./protocol/LoginProtocol.cpp
    ThreadPool.cpp
    ./protocol/PacketCodec.h
)

# Linux 下链接必要的系统库
if(UNIX)
    target_link_libraries(login_server
        pthread   # 多线程
    )
endif()

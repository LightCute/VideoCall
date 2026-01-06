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
    # 根目录文件
    server_main.cpp

    # core目录文件
    ./core/LoginServer.cpp
    ./core/LoginServer.h

    # dispatcher目录文件
    ./dispatcher/ServerEventDispatcher.cpp
    ./dispatcher/ServerEventDispatcher.h

    # event目录文件
    ./event/ServerEvent.h
    ./event/ServerEventFactory.cpp
    ./event/ServerEventFactory.h

    # net目录文件
    ./net/CommandSocket.cpp
    ./net/CommandSocket.h
    ./net/PacketCodec.h

    # protocol目录文件
    ./protocol/protocol_common.h
    ./protocol/protocol_text.cpp
    ./protocol/protocol_text.h
    ./protocol/protocol_types.h

    # service目录文件
    ./service/LoginService.cpp
    ./service/LoginService.h
    ./service/ServerAction.h

    # session目录文件
    ./session/SessionManager.cpp
    ./session/SessionManager.h

    # util目录文件
    ./util/ThreadPool.cpp
    ./util/ThreadPool.h
)

# Linux 下链接必要的系统库
if(UNIX)
    target_link_libraries(login_server
        pthread   # 多线程
    )
endif()

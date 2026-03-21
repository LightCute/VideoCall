cmake_minimum_required(VERSION 3.10) # 参考成功案例，稍微提高版本要求

project(datachannel_test_3_20_recv VERSION 0.1 LANGUAGES CXX)

# =========================
# 基础配置
# =========================
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 帮助 IDE 索引

# =========================
# 第三方库：LibDataChannel 等
# =========================
add_subdirectory(third_party/libdatachannel)

# spdlog
add_library(spdlog_header_only INTERFACE)
target_include_directories(spdlog_header_only INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/include
)

find_package(Threads REQUIRED)

# =========================
# 第三方库：GStreamer (参考成功案例的写法)
# =========================
find_package(PkgConfig REQUIRED)

# ✅ 1. 查找模块 (去掉 IMPORTED_TARGET，使用最原始的方式)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-app-1.0
)

# =========================
# 源文件
# =========================
set(PROJECT_SOURCES
    main.cpp
    utilities/log.h
    DataChannelClient.cpp
    DataChannelClient.h
    dispatchqueue.cpp
    dispatchqueue.hpp
    h264fileparser.cpp
    h264fileparser.hpp
    helpers.cpp
    helpers.hpp
    opusfileparser.cpp
    opusfileparser.hpp
    fileparser.cpp
    fileparser.hpp
    stream.cpp
    stream.hpp
    ArgParser.cpp
    ArgParser.hpp
    
    # ✅ 2. 新增的文件 (确保 CMake 能看到它们)
    ThreadSafeQueue.h
    GstMediaPlayer.cpp
	GstMediaPlayer.h
)

# =========================
# 可执行文件
# =========================
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)

# =========================
# Include 路径 (参考成功案例，显式添加)
# =========================
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}          # 让 #include "GstVideoPlayer.h" 能找到
        ${GST_INCLUDE_DIRS}                 # ✅ 关键：显式添加 GStreamer 头文件路径
)

# =========================
# 链接库
# =========================
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        LibDataChannel::LibDataChannel
        Threads::Threads
        nlohmann_json::nlohmann_json
        spdlog_header_only
        ${GST_LIBRARIES}                    # ✅ 关键：显式链接 GStreamer 库
)

# =========================
# 链接选项 (可选：防止某些环境下找不到库)
# =========================
target_link_directories(${PROJECT_NAME}
    PRIVATE
        ${GST_LIBRARY_DIRS}
)
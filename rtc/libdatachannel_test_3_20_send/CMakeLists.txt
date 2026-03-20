cmake_minimum_required(VERSION 3.5)

# 项目基础配置（移除Qt版本/UI相关）
project(datachannel_test VERSION 0.1 LANGUAGES CXX)

# 导出编译命令（方便IDE索引）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++标准（LibDataChannel需要C++17+）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===================== 第三方库配置 =====================
# 1. 编译LibDataChannel（假设third_party/libdatachannel是源码目录）
add_subdirectory(third_party/libdatachannel)

# 2. spdlog（头文件仅模式，无需编译）
add_library(spdlog_header_only INTERFACE)
target_include_directories(spdlog_header_only INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/include
)

# 3. nlohmann_json（用户代码中用到了JSON解析，需添加，若无则删除）
# 方式1：如果有源码目录（third_party/nlohmann_json）
# add_subdirectory(third_party/nlohmann_json)
# 方式2：通过find_package（已安装到系统）

# 4. 线程库（LibDataChannel依赖）
find_package(Threads REQUIRED)

# ===================== 源文件配置 =====================
# 仅保留非Qt源文件（删除widget相关的ui/h/cpp）
set(PROJECT_SOURCES
    main.cpp       # 主函数（已移除Qt代码）
    utilities/log.h # spdlog封装
    utilities/blocking_queue.h # 可选，若用到则保留
    DataChannelClient.cpp # 数据通道客户端实现（已移除Qt相关代码）
    DataChannelClient.h   # 数据通道客户端头文件（已移除Qt相关代码）
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
)

set(STREAMER_UWP_RESOURCES
	uwp/Logo.png
	uwp/package.appxManifest
	uwp/SmallLogo.png
	uwp/SmallLogo44x44.png
	uwp/SplashScreen.png
	uwp/StoreLogo.png
	uwp/Windows_TemporaryKey.pfx
)

# ===================== 可执行文件 =====================
# 普通控制台可执行程序（无Qt的MACOSX/WIN32打包）
add_executable(datachannel_test_3_20_send
    ${PROJECT_SOURCES}
)

# ===================== 链接依赖 =====================
target_link_libraries(datachannel_test_3_20_send PRIVATE
    LibDataChannel::LibDataChannel  # LibDataChannel核心库
    Threads::Threads                # 线程库（必须）
    nlohmann_json::nlohmann_json    # JSON库（代码中用到，删除则注释）
    spdlog_header_only              # spdlog头文件库
)

# ===================== 安装配置（可选精简） =====================
include(GNUInstallDirs)
install(TARGETS datachannel_test_3_20_send
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # 可执行文件安装路径
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}  # 库文件（若有）
)
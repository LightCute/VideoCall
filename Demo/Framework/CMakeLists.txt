cmake_minimum_required(VERSION 3.16)
project(app CXX)

# 使用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 源文件
set(SRC
    main.cpp
    # 其他 cpp 文件，如果有
    # ...
)

# spdlog header-only 路径
add_library(spdlog_header_only INTERFACE)
target_include_directories(spdlog_header_only INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/include
)
# 直接定义 SPDLOG_HEADER_ONLY
target_compile_definitions(spdlog_header_only INTERFACE SPDLOG_HEADER_ONLY)

# 可执行文件
add_executable(${PROJECT_NAME} ${SRC})

# 链接 spdlog header-only
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog_header_only)
cmake_minimum_required(VERSION 3.5)

project(VideoCall VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(third_party/libdatachannel)

add_library(spdlog_header_only INTERFACE)
target_include_directories(spdlog_header_only INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/include
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Threads REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(EVENT_DRIVEN_SOURCES
    # 应用层
    src/application/app_context.h

    # 适配层
    src/adapter/websocket.h
    src/adapter/websocket.cpp

    # 服务层抽象
    src/service/abstract_net.h
    src/service/abstract_ui.h

    # 工具层
    src/utilities/blocking_queue.h
    src/utilities/log.h

    # 框架层 - 核心
    src/framework/core/core.h

    # 框架层 - 命令
    src/framework/command/abstract_command.h
    src/framework/command/abstract_command_dispatcher.h
    src/framework/command/abstract_receiver.h
    src/framework/command/command_context.h

    # 框架层 - 事件
    src/framework/event/abstract_event.h
    src/framework/event/abstract_event_listener.h
    src/framework/event/event_bus.h
    src/framework/event/event_router.h

    # 框架层 - 状态
    src/framework/state/abstract_call_state.h
    src/framework/state/abstract_login_state.h
    src/framework/state/abstract_state.h

    # 框架层 - 会话
    src/framework/session/abstract_session.h
    src/framework/session/session_manager.h
)

set(BUSINESS_LOGIN_SOURCES
    # 业务层 - 登录
    src/business/login/receiver/login_receiver.h
    src/business/login/state/logged_in_state.h
    src/business/login/state/logged_out_state.h
    src/business/login/state/logging_in_state.h
    src/business/login/session/login_session.h
    src/business/login/command/login_command.h
    src/business/login/event/login_event.h
    src/business/login/event/login_failed_event.h
    src/business/login/event/login_success_event.h
    src/business/login/model/login_payload.h
)

set(BUSINESS_CONNECT_SOURCES
    # 业务层 - 连接
    src/business/connect/receiver/connect_receiver.h
    src/business/connect/state/connected_state.h       # 新增：已连接状态
    src/business/connect/state/connecting_state.h      # 原有：连接中状态
    src/business/connect/state/disconnected_state.h    # 原有：断开连接状态
    src/business/connect/session/connect_session.h
    src/business/connect/command/connect_command.h
    src/business/connect/event/connect_failed_event.h  # 新增：连接失败事件
    src/business/connect/event/connect_server_event.h  # 原有：连接请求事件
    src/business/connect/event/connect_success_event.h # 新增：连接成功事件
    src/business/connect/model/connect_payload.h
)

set(BUSINESS_CALL_SOURCES
    # 业务层 - 通话
    src/business/call/state/call_idle_state.h
    src/business/call/state/calling_state.h
    src/business/call/session/call_session.h
    src/business/call/event/call_event.h
)

set(PROJECT_SOURCES
        main.cpp
        ${EVENT_DRIVEN_SOURCES}
        ${BUSINESS_LOGIN_SOURCES}
        ${BUSINESS_CONNECT_SOURCES}
        ${BUSINESS_CALL_SOURCES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(VideoCall
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET VideoCall APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(VideoCall SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(VideoCall
            ${PROJECT_SOURCES}
            src/adapter/qt_ui.h
            src/adapter/qt_ui.cpp
            src/adapter/qt_ui.ui
        )
    endif()
endif()

target_link_libraries(VideoCall PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    LibDataChannel::LibDataChannel          # libdatachannel核心库
    Threads::Threads                        # 线程库依赖
    nlohmann_json::nlohmann_json            # JSON库依赖（不用可删除）
    spdlog_header_only

)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.VideoCall)
endif()
set_target_properties(VideoCall PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS VideoCall
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(VideoCall)
endif()

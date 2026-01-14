cmake_minimum_required(VERSION 3.10)

project(qt_gst_camera
    VERSION 0.1
    LANGUAGES CXX
)

# =========================
# 基础配置
# =========================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# =========================
# Qt
# =========================
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# =========================
# GStreamer
# =========================
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-app-1.0
)


# =========================
# 所有工程源文件（统一管理）
# =========================
set(PROJECT_SOURCES
    main.cpp

    #core
    core/ClientCore.cpp
    core/ClientCore.h
    core/ClientState.h
    core/CoreInput.h
    core/CoreOutput.h
    core/FSM.h
    core/FSM.cpp
    core/CoreExecutor.h
    core/CoreExecutor.cpp


    # app
    app/ClientEvent.h
    app/ClientEventFactory.cpp
    app/ClientEventFactory.h
    app/ClientEventEnum.h


    # media
    media/CameraManager.cpp
    media/CameraManager.h
    media/Frame.h

    # net
    net/CommandSocket.cpp
    net/CommandSocket.h
    net/PacketCodec.h

    # protocol
    protocol/protocol_common.h
    protocol/protocol_text.cpp
    protocol/protocol_text.h
    protocol/protocol_types.h

    # ui
    ui/AppWindow.cpp
    ui/AppWindow.h
    ui/LoginWidget.cpp
    ui/LoginWidget.h
    ui/QtCameraAdapter.cpp
    ui/QtCameraAdapter.h
    ui/VideoWidget.cpp
    ui/VideoWidget.h
    ui/widget.cpp
    ui/widget.h

    # Qt UI files
    ui/LoginWidget.ui
    ui/widget.ui
)


# =========================
# 可执行程序
# =========================
if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        #media/CameraManager.h


    )
endif()

# =========================
# Include 路径
# =========================
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${GST_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/app
        ${CMAKE_CURRENT_SOURCE_DIR}/media
        ${CMAKE_CURRENT_SOURCE_DIR}/net
        ${CMAKE_CURRENT_SOURCE_DIR}/protocol
        ${CMAKE_CURRENT_SOURCE_DIR}/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/core
)

# =========================
# 链接库
# =========================
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        ${GST_LIBRARIES}
)

# =========================
# Qt6 finalize
# =========================
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()

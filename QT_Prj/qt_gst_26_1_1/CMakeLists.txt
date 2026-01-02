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
# 源文件分组（工程级推荐）
# =========================

# ---- UI (Qt) ----
set(UI_SOURCES
    main.cpp
    widget.cpp
    widget.h
    widget.ui
    VideoWidget.cpp
    VideoWidget.h
    QtCameraAdapter.cpp
    QtCameraAdapter.h
)

# ---- Core (Pure C++) ----
set(CORE_SOURCES
    CameraManager.cpp
    CameraManager.h
    Frame.h
)

# =========================
# 可执行程序
# =========================
if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${UI_SOURCES}
        ${CORE_SOURCES}
    )
else()
    add_executable(${PROJECT_NAME}
        ${UI_SOURCES}
        ${CORE_SOURCES}
        CommandSocket.h CommandSocket.cpp
        LoginWidget.h LoginWidget.cpp LoginWidget.ui
        AppWindow.h AppWindow.cpp

    )
endif()

# =========================
# Include 路径
# =========================
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${GST_INCLUDE_DIRS}
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

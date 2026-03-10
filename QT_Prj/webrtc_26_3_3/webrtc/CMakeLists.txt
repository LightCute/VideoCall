cmake_minimum_required(VERSION 3.5)

project(webrtc VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(3rdparty/libdatachannel)
add_subdirectory(3rdparty/eventpp)

add_library(spdlog_header_only INTERFACE)
target_include_directories(spdlog_header_only INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog/include
)
#target_compile_definitions(spdlog_header_only INTERFACE SPDLOG_HEADER_ONLY)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Threads REQUIRED)          # 线程库（libdatachannel必需）

set(PROJECT_SOURCES
        main.cpp
        src/ui/widget.cpp
        src/ui/widget.h
        src/ui/widget.ui
        src/net/Websocket.h
        src/net/Websocket.cpp
        src/net/INet.h
        src/core/core.h
        src/core/core.cpp
        src/core/IEventQueue.h
        src/core/IEventData.h
        src/core/ConnectEvent.h
        src/ui/IUI.h
        src/utilities/log.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(webrtc
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET webrtc APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(webrtc SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(webrtc
            ${PROJECT_SOURCES}
        )
    endif()
endif()


target_link_libraries(webrtc PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets          # 原有Qt链接
    LibDataChannel::LibDataChannel          # libdatachannel核心库
    Threads::Threads                        # 线程库依赖
    nlohmann_json::nlohmann_json            # JSON库依赖（不用可删除）
    eventpp::eventpp
    spdlog_header_only
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.webrtc)
endif()
set_target_properties(webrtc PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS webrtc
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(webrtc)
endif()

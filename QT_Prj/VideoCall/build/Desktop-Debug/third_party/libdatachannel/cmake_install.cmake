# Install script for directory: /home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/llvm-objdump-18")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdatachannel.so.0.24.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdatachannel.so.0.24"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/libdatachannel.so.0.24.1"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/libdatachannel.so.0.24"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdatachannel.so.0.24.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdatachannel.so.0.24"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/llvm-strip-18" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/libdatachannel.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rtc" TYPE FILE FILES
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/candidate.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/channel.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/configuration.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/datachannel.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/dependencydescriptor.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/description.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/iceudpmuxlistener.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/mediahandler.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/rtcpreceivingsession.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/common.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/global.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/message.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/frameinfo.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/peerconnection.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/reliability.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/rtc.h"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/rtc.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/rtp.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/track.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/websocket.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/websocketserver.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/rtppacketizationconfig.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/rtcpsrreporter.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/rtppacketizer.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/rtpdepacketizer.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/h264rtppacketizer.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/h264rtpdepacketizer.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/nalunit.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/h265rtppacketizer.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/h265rtpdepacketizer.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/h265nalunit.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/av1rtppacketizer.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/vp8rtppacketizer.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/vp8rtpdepacketizer.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/rtcpnackresponder.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/utils.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/plihandler.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/pacinghandler.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/rembhandler.hpp"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/third_party/libdatachannel/include/rtc/version.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel/LibDataChannelTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel/LibDataChannelTargets.cmake"
         "/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/CMakeFiles/Export/32c821eb1e7b36c3a3818aec162f7fd2/LibDataChannelTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel/LibDataChannelTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel/LibDataChannelTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel" TYPE FILE FILES "/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/CMakeFiles/Export/32c821eb1e7b36c3a3818aec162f7fd2/LibDataChannelTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel" TYPE FILE FILES "/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/CMakeFiles/Export/32c821eb1e7b36c3a3818aec162f7fd2/LibDataChannelTargets-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibDataChannel" TYPE FILE FILES
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/LibDataChannelConfig.cmake"
    "/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/LibDataChannelConfigVersion.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/examples/client/cmake_install.cmake")
  include("/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/examples/client-benchmark/cmake_install.cmake")
  include("/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/examples/media-receiver/cmake_install.cmake")
  include("/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/examples/media-sender/cmake_install.cmake")
  include("/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/examples/media-sfu/cmake_install.cmake")
  include("/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/examples/streamer/cmake_install.cmake")
  include("/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/examples/copy-paste/cmake_install.cmake")
  include("/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/examples/copy-paste-capi/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/light/work/Video_Prj/QT_Prj/VideoCall/build/Desktop-Debug/third_party/libdatachannel/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()

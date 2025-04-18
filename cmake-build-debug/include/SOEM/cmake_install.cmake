# Install script for directory: /home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/install")
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
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/surrogate/Jaehyun/github_repos/surrogate_repo/cmake-build-debug/include/SOEM/libsoem.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/surrogate/Jaehyun/github_repos/surrogate_repo/cmake-build-debug/include/SOEM/CMakeFiles/soem.dir/install-cxx-module-bmi-Debug.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig.cmake"
         "/home/surrogate/Jaehyun/github_repos/surrogate_repo/cmake-build-debug/include/SOEM/CMakeFiles/Export/39806c66e6e7fd9076eb39407f12ee6f/soemConfig.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/soem/cmake" TYPE FILE FILES "/home/surrogate/Jaehyun/github_repos/surrogate_repo/cmake-build-debug/include/SOEM/CMakeFiles/Export/39806c66e6e7fd9076eb39407f12ee6f/soemConfig.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/soem/cmake" TYPE FILE FILES "/home/surrogate/Jaehyun/github_repos/surrogate_repo/cmake-build-debug/include/SOEM/CMakeFiles/Export/39806c66e6e7fd9076eb39407f12ee6f/soemConfig-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/soem" TYPE FILE FILES
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercat.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercatbase.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercatcoe.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercatconfig.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercatconfiglist.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercatdc.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercateoe.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercatfoe.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercatmain.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercatprint.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercatsoe.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/soem/ethercattype.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/osal/linux/osal_defs.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/osal/osal.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/oshw/linux/nicdrv.h"
    "/home/surrogate/Jaehyun/github_repos/surrogate_repo/include/SOEM/oshw/linux/oshw.h"
    )
endif()


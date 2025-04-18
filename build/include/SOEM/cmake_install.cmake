# Install script for directory: /home/surrogate/Desktop/MARCH_Surrogate/include/SOEM

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

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/surrogate/Desktop/MARCH_Surrogate/build/include/SOEM/libsoem.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig.cmake"
         "/home/surrogate/Desktop/MARCH_Surrogate/build/include/SOEM/CMakeFiles/Export/share/soem/cmake/soemConfig.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/soem/cmake" TYPE FILE FILES "/home/surrogate/Desktop/MARCH_Surrogate/build/include/SOEM/CMakeFiles/Export/share/soem/cmake/soemConfig.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/soem/cmake" TYPE FILE FILES "/home/surrogate/Desktop/MARCH_Surrogate/build/include/SOEM/CMakeFiles/Export/share/soem/cmake/soemConfig-debug.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/soem" TYPE FILE FILES
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercat.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercatbase.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercatcoe.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercatconfig.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercatconfiglist.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercatdc.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercateoe.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercatfoe.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercatmain.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercatprint.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercatsoe.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/soem/ethercattype.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/osal/linux/osal_defs.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/osal/osal.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/oshw/linux/nicdrv.h"
    "/home/surrogate/Desktop/MARCH_Surrogate/include/SOEM/oshw/linux/oshw.h"
    )
endif()


PROJECT(Surrogate C CXX)
# cmake_policy(SET CMP0015 NEW)
# cmake_policy(SET CMP0048 NEW)

set(CMAKE_CXX_STANDARD 17)
cmake_minimum_required(VERSION 3.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# find Xenomai package
find_package(Xenomai REQUIRED)
if(Xenomai_FOUND)
    message("Xenomai package was found")
endif(Xenomai_FOUND)

link_directories(
        "/usr/xenomai/lib"
)

set(xeno_cflags_params "--posix" "--alchemy" "--cflags") 
execute_process(
        COMMAND xeno-config ${xeno_cflags_params}
        OUTPUT_VARIABLE xeno_cflags
        OUTPUT_STRIP_TRAILING_WHITESPACE)

set(xeno_ldflags_params "--posix" "--alchemy" "--ldflags")
execute_process(
        COMMAND xeno-config ${xeno_ldflags_params}
        OUTPUT_VARIABLE xeno_ldflags
        OUTPUT_STRIP_TRAILING_WHITESPACE)

#set(CMAKE_C_COMPILER g++)
set(CMAKE_C_FLAGS          "${CMAKE_CXX_FLAGS} ${xeno_cflags}")
set(CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS} ${xeno_cflags}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${xeno_ldflags}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${xeno_ldflags}")

# build SOEM library
add_subdirectory(include/SOEM)

# add gRPC library
set(target_proto_names
        "control"
        "motion"
        "system"
        "msgs_common"
        "msgs_config"
        "msgs_control"
        "msgs_motion"
        "msgs_system"
)
include(include/gRPC/cpp/cmake/common.cmake)
add_subdirectory(include/gRPC)
include_directories("include/gRPC/cpp/proto_generated")

set(target_grpc_protos)
foreach (target_proto_name ${target_proto_names})
    list(APPEND target_grpc_protos "${target_proto_name}_grpc_proto")
endforeach ()

add_library(custom_grpc_server SHARED include/gRPC/cpp/custom_server.cpp include/gRPC/cpp/custom_grpc_service.cpp)
target_link_libraries(custom_grpc_server PUBLIC
        ${target_grpc_protos}
        absl::check
        absl::flags
        absl::flags_parse
        absl::log
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
)

add_library(custom_shared_memory STATIC include/SharedMemory/shared_memory.h)
set_target_properties(custom_shared_memory PROPERTIES LINKER_LANGUAGE CXX)
# build sources
set(LIBS alchemy cobalt copperplate trank soem custom_grpc_server)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/example_xenomai)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Test_Wheel)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Test_Dynamixel)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/test_integration)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Test_Balancer)

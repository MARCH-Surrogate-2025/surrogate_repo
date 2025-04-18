# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ helloworld example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building helloworld.

cmake_minimum_required(VERSION 3.8)

project(CustomRPC C CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/common.cmake)

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
set(target_generate_dir "${CMAKE_CURRENT_SOURCE_DIR}/proto_generated")
file(MAKE_DIRECTORY ${target_generate_dir})
include_directories("${target_generate_dir}")

# Proto file
foreach(target_proto_name ${target_proto_names})
  get_filename_component(target_proto "../protos/${target_proto_name}.proto" ABSOLUTE)
  get_filename_component(target_proto_path "${target_proto}" PATH)

  # Generated sources
  set(target_proto_srcs "${target_generate_dir}/${target_proto_name}.pb.cc")
  set(target_proto_hdrs "${target_generate_dir}/${target_proto_name}.pb.h")
  set(target_grpc_srcs "${target_generate_dir}/${target_proto_name}.grpc.pb.cc")
  set(target_grpc_hdrs "${target_generate_dir}/${target_proto_name}.grpc.pb.h")

  add_custom_command(
        OUTPUT "${target_proto_srcs}" "${target_proto_hdrs}" "${target_grpc_srcs}" "${target_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${target_generate_dir}"
          --cpp_out "${target_generate_dir}"
          -I "${target_proto_path}"
          --proto_path "${target_proto_path}"
          --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
          "${target_proto}"
        DEPENDS "${target_proto}")

  # target_grpc_proto
  add_library(${target_proto_name}_grpc_proto STATIC ${target_grpc_srcs} ${target_grpc_hdrs} ${target_proto_srcs} ${target_proto_hdrs})
  target_link_libraries(${target_proto_name}_grpc_proto absl::check ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})
endforeach()

set(target_grpc_protos)
foreach (target_proto_name ${target_proto_names})
  list(APPEND target_grpc_protos "${target_proto_name}_grpc_proto")
endforeach ()

# # Targets greeter_[async_](client|server)
# foreach(_target custom_client)
#   add_executable(${_target} "${_target}.cc")
#   target_link_libraries(${_target}
#           ${target_grpc_protos}
#           absl::check
#           absl::flags
#           absl::flags_parse
#           absl::log
#           ${_REFLECTION}
#           ${_GRPC_GRPCPP}
#           ${_PROTOBUF_LIBPROTOBUF})
# endforeach()
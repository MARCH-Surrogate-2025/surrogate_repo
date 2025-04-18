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
set(target_generate_dir "${CMAKE_CURRENT_SOURCE_DIR}/cpp/proto_generated")
# Proto file
foreach(target_proto_name ${target_proto_names})

  set(target_proto_srcs "${target_generate_dir}/${target_proto_name}.pb.cc")
  set(target_proto_hdrs "${target_generate_dir}/${target_proto_name}.pb.h")
  set(target_grpc_srcs "${target_generate_dir}/${target_proto_name}.grpc.pb.cc")
  set(target_grpc_hdrs "${target_generate_dir}/${target_proto_name}.grpc.pb.h")

  # Include generated *.pb.h files
  include_directories("${target_generate_dir}")

  # target_grpc_proto
  add_library(${target_proto_name}_grpc_proto SHARED
    ${target_grpc_srcs}
    ${target_grpc_hdrs}
    ${target_proto_srcs}
    ${target_proto_hdrs})
  target_link_libraries(${target_proto_name}_grpc_proto
    absl::check
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})

endforeach()
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

cmake_minimum_required(VERSION 2.8)

project(timemachine C CXX)

SET(CMAKE_CXX_STANDARD 11)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    add_definitions(-D_WIN32_WINNT=0x600)
endif()

find_package(AWSSDK REQUIRED COMPONENTS s3 kinesis)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
# This branch assumes that gRPC and all its dependencies are already installed
# on this system, so they can be located by find_package().

# Find Protobuf installation
# Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

find_package(BZip2 REQUIRED)
find_package(Snappy REQUIRED)


find_program(GRPC_PLUGIN NAMES "grpc_cpp_plugin")

if (NOT EXISTS ${GRPC_PLUGIN})
    message(WARNING "The grpc_${GEN_LANG}_plugin plugin was not found, \
                   the gRPC classes are not being generated")
else(EXISTS ${GRPC_PLUGIN})
    message(STATUS "Found grpc_${GEN_LANG}_plugin : " ${GRPC_PLUGIN})
endif()

set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure)

# Proto file
get_filename_component(tm_proto "proto/timeMachine.proto" ABSOLUTE)
get_filename_component(tm_proto_path "${tm_proto}" PATH)

# Generated sources
set(tm_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/timeMachine.pb.cc")
set(tm_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/timeMachine.pb.h")
set(tm_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/timeMachine.grpc.pb.cc")
set(tm_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/timeMachine.grpc.pb.h")
add_custom_command(
        OUTPUT "${tm_proto_srcs}" "${tm_proto_hdrs}" "${tm_grpc_srcs}" "${tm_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${tm_proto_path}"
        --plugin=protoc-gen-grpc="${GRPC_PLUGIN}"
        "${tm_proto}"
        DEPENDS "${tm_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Targets greeter_[async_](client|server)

find_library(ROCKSDB_LIBRARY rocksdb)

add_executable(timemachine "timemachine.cpp" Server.cpp Server.h
        ${tm_proto_srcs}
        ${tm_grpc_srcs})
target_link_libraries(timemachine
        ${BZIP2_LIBRARIES}
        ${SNAPPY_LIBRARIES}
        ${_GRPC_GRPCPP_UNSECURE}
        ${AWSSDK_LINK_LIBRARIES}
        ${ROCKSDB_LIBRARY}
        ${_PROTOBUF_LIBPROTOBUF})



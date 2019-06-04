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

project(reqstore C CXX)

SET(CMAKE_CXX_STANDARD 14)

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

message(STATUS "Looking for POCO")
find_library(POCO_FOUNDATION PocoFoundation)
find_library(POCO_UTIL PocoUtil)
find_library(POCO_NET PocoNet)
find_library(POCO_JSON PocoJSON)
find_library(POCO_XML PocoXML)


find_package(BZip2 REQUIRED)
find_package(Snappy REQUIRED)

message(STATUS "Looking for spdlog")
find_package(spdlog REQUIRED)

ExternalProject_Add(protos
        GIT_REPOSITORY https://github.com/Hydrospheredata/hydro-serving-protos.git
        GIT_TAG "origin/subsampling"
        PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/proto"
        CONFIGURE_COMMAND "" # Disable configure step
        BUILD_COMMAND "" # Disable build step
        INSTALL_COMMAND "" # Disable install step
        UPDATE_COMMAND "" # Disable update step: clones the project only once
        LOG_DOWNLOAD ON
        )

set(SPBLOG spdlog::spblog)
#set(SPBLOG_SINKS_COLOR spdlog)


find_program(GRPC_PLUGIN NAMES "grpc_cpp_plugin")

if (NOT EXISTS ${GRPC_PLUGIN})
    message(WARNING "The grpc_${GEN_LANG}_plugin plugin was not found, \
                   the gRPC classes are not being generated")
else(EXISTS ${GRPC_PLUGIN})
    message(STATUS "Found grpc_${GEN_LANG}_plugin : " ${GRPC_PLUGIN})
endif()

set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure)

# Proto file
get_filename_component(tm_proto "proto/src/protos/src/hydro_serving_grpc/reqstore/reqstore_service.proto" ABSOLUTE)
get_filename_component(tm_proto_path "${tm_proto}" PATH)

# Generated sources
set(tm_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/reqstore_service.pb.cc")
set(tm_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/reqstore_service.pb.h")
set(tm_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/reqstore_service.grpc.pb.cc")
set(tm_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/reqstore_service.grpc.pb.h")
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

include_directories("${spdlog_INCLUDE_DIR}")

# Targets greeter_[async_](client|server)


find_library(ROCKSDB_LIBRARY rocksdb)

add_executable(reqstore "reqstore.cpp" 
        GRPCServer.cpp GRPCServer.h
        CloudDBClient.h CloudDBClient.cpp
        LocalDBClient.h LocalDBClient.cpp
        Config.cpp Config.h
        IDComparator.h IDComparator.cpp
        DbClient.h DbClient.cpp
        HTTPServer.h HTTPServer.cpp
        handlers/HandlerFactory.h handlers/HandlerFactory.cpp
        ${tm_proto_srcs}
        ${tm_grpc_srcs} handlers/HealthHandler.cpp handlers/HealthHandler.h
        handlers/InfoHandler.cpp handlers/InfoHandler.h
        handlers/StatusHandler.cpp handlers/StatusHandler.h
        handlers/SaveHandler.cpp handlers/SaveHandler.h
        utils/RepositoryUtils.cpp utils/RepositoryUtils.h
        handlers/NotFoundHandler.cpp handlers/NotFoundHandler.h
        handlers/GetRangeHandler.cpp handlers/GetRangeHandler.h)
target_link_libraries(reqstore
        ${BZIP2_LIBRARIES}
        ${POCO_UTIL}
        ${POCO_NET}
        ${POCO_JSON}
        ${POCO_XML}
        ${POCO_FOUNDATION}
        ${CMAKE_THREAD_LIBS_INIT}
        ${SNAPPY_LIBRARIES}
        ${_GRPC_GRPCPP_UNSECURE}
        ${AWSSDK_LINK_LIBRARIES}
        ${ROCKSDB_LIBRARY}
        ${_PROTOBUF_LIBPROTOBUF})



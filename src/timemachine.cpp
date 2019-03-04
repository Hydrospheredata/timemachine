#include <iostream>
#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "timeMachine.grpc.pb.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "Server.h"

#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include "Config.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/async.h"

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


using namespace rocksdb;

using rocksdb::DB;
using rocksdb::Options;

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;

using timemachine::Empty;
using timemachine::Timemachine;
using grpc::ServerContext;
using timemachine::Range;
using grpc::ServerWriter;
using timemachine::Data;
using timemachine::ID;
using timemachine::Empty;
using timemachine::TSServer;

using DataWriter = ServerWriter<Data>;

void initLogger(spdlog::level::level_enum logLevel)
{
        spdlog::init_thread_pool(8192, 1);
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
        stdout_sink->set_level(logLevel);
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("timemachine.log", 1024*1024*10, 3);
        rotating_sink->set_level(logLevel);
        std::vector<spdlog::sink_ptr> sinks {stdout_sink, rotating_sink};
        auto logger = std::make_shared<spdlog::async_logger>("log", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        logger->set_level(logLevel);
        logger->set_pattern("%^[%l][%H:%M:%S %z] [thread %t] %v");
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
}

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

void RunServer() {
    try {
        auto conf = timemachine::Config();
        auto cfg = std::make_shared<timemachine::Config>(conf);
        initLogger(cfg->debug ? spdlog::level::debug : spdlog::level::info);
        spdlog::info("Initializing config: {}", cfg->ToString());
        auto dbClient = std::make_shared<timemachine::DbClient>(timemachine::DbClient(cfg));
        spdlog::info("dbClient initialized sucessfully");
        std::string server_address("0.0.0.0:8080");
        TSServer service;
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        service.Init(dbClient);
        spdlog::info("Server listening on {0}", server_address);
        server->Wait();
        spdlog::info("Server is down: {0}", server_address);
    } catch (const std::exception &e) {
        spdlog::error("Something went wrong: {}. Closing server", e.what());
    }
    catch (...) {
        spdlog::error("Something went wrong. Closing server");
    }
}

int main() {
    signal(SIGSEGV, handler); 
    RunServer();
    return 0;
};




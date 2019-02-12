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

void RunServer() {
    try {
        DBCloud *db;

        auto baseName = "timemachine";

        CloudEnvOptions cloud_env_options;
        std::unique_ptr<CloudEnv> cloud_env;

        char *keyid = getenv("AWS_ACCESS_KEY_ID");
        char *secret = getenv("AWS_SECRET_ACCESS_KEY");
        char *bucketName = getenv("AWS_BUCKET_NAME");
        char *kRegion = getenv("AWS_DEFAULT_REGION");

        if (keyid == nullptr || secret == nullptr || bucketName == nullptr || kRegion == nullptr) {
            fprintf(
                    stderr,
                    "Please set env variables "
                    "AWS_BUCKET_NAME, AWS_DEFAULT_REGION, AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY with cloud credentials");
            throw;
        }
        cloud_env_options.credentials.access_key_id.assign(keyid);
        cloud_env_options.credentials.secret_key.assign(secret);
        cloud_env_options.log_type = rocksdb::LogType::kLogKinesis;

        CloudEnv *cenv;
        rocksdb::Status s =
                CloudEnv::NewAwsEnv(Env::Default(),
                                    bucketName, baseName, kRegion,
                                    bucketName, baseName, kRegion,
                                    cloud_env_options, nullptr, &cenv);
        if (!s.ok()) {
            fprintf(stderr, "Unable to create cloud env in bucket %s. %s\n",
                    bucketName, s.ToString().c_str());
            throw;
        }
        cloud_env.reset(cenv);

        Options options;
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        options.env = cloud_env.get();
        options.create_if_missing = true;

        std::string persistent_cache = "";


        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        options.create_if_missing = true;

        s = DBCloud::Open(options, baseName, persistent_cache, 0, &db);

        std::cout << "caching connection" << std::endl;

        if (!s.ok()) {
            fprintf(stderr, "Unable to open db at path %s with bucket %s. %s\n",
                    baseName, bucketName, s.ToString().c_str());
            throw;
        }

        std::cout << "DB is opened: " << s.ToString() << std::endl;
        std::string server_address("0.0.0.0:8080");
        TSServer service;
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        service.db = db;
        std::cout << "Server listening on " << server_address << std::endl;
        server->Wait();
        std::cout << "Server is down: " << server_address << std::endl;
    } catch (...) {
        std::cerr << "SOmething went wrong. Closing server" << std::endl;
    }
}

int main() {
    RunServer();
    return 0;
};


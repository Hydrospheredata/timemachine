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

        CloudEnvOptions cloud_env_options;
        std::unique_ptr<CloudEnv> cloud_env;

        char *keyid = getenv("AWS_ACCESS_KEY_ID");
        char *secret = getenv("AWS_SECRET_ACCESS_KEY");
        char *kRegion = getenv("AWS_DEFAULT_REGION");
        char *walProvider = getenv("WAL_PROVIDER");
        char *sourceLocalDir = getenv("SRC_LOCAL_DIR");
        char *destinationLocalDir = getenv("DST_LOCAL_DIR");
        char *sourceBucket = getenv("SRC_BUCKET");
        char *destBucket = getenv("DST_BUCKET");
        char *dbName = getenv("DB_NAME");

        if(sourceLocalDir == nullptr){
            sourceLocalDir = (char *)"default-timemachine";
        }

        if(destinationLocalDir == nullptr){
            destinationLocalDir = (char *)"default-timemachine";
        }

        if(sourceBucket == nullptr){
            sourceBucket = (char *)"default-timemachine";
        }

        if(destBucket == nullptr){
            destBucket = (char *)"default-timemachine";
        }

        bool useKinesis = false;

        auto kinesisString = "kinesis";

        if(walProvider != nullptr && strcmp(walProvider, kinesisString) == 0){
            useKinesis = true;
            std::cout << "starting with kinesis WAL" << std::endl;
        } else {
            std::cout << "starting with local WAL" << std::endl;
        }

        if (keyid == nullptr || secret == nullptr  || kRegion == nullptr || dbName == nullptr) {
            fprintf(
                    stderr,
                    "Please set env variables "
                    "AWS_BUCKET_NAME, AWS_DEFAULT_REGION, AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY with cloud credentials");
            throw;
        }
        cloud_env_options.credentials.access_key_id.assign(keyid);
        cloud_env_options.credentials.secret_key.assign(secret);
        if(useKinesis){
            cloud_env_options.keep_local_log_files = false;
            cloud_env_options.log_type = LogType::kLogKinesis;
        }


        CloudEnv *cenv;
        rocksdb::Status s =
                CloudEnv::NewAwsEnv(Env::Default(),
                                    sourceBucket, sourceLocalDir, kRegion,
                                    destBucket, destinationLocalDir, kRegion,
                                    cloud_env_options, nullptr, &cenv);
        if (!s.ok()) {
            fprintf(stderr, "Unable to create cloud env in bucket %s. %s\n",
                    sourceBucket, s.ToString().c_str());
            throw;
        }
        cloud_env.reset(cenv);

        Options options;
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        options.env = cloud_env.get();
        options.create_if_missing = true;

        std::string persistent_cache = "";


        s = DBCloud::Open(options, dbName, persistent_cache, 0, &db);

        std::cout << "caching connection" << std::endl;

        if (!s.ok()) {
            fprintf(stderr, "Unable to open db at path %s with bucket %s. %s\n",
                    dbName, sourceBucket, s.ToString().c_str());
            throw;
        }

        std::cout << "DB is opened: " << dbName << " - " << s.ToString() << std::endl;
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
        std::cerr << "Something went wrong. Closing server" << std::endl;
    }
}

int main() {
    RunServer();
    return 0;
};


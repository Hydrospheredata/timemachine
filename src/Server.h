//
// Created by Dmitry Isaev on 2019-01-30.
//

#include "timeMachine.grpc.pb.h"
#include <functional>
#include "DbClient.h"

using timemachine::Empty;
using timemachine::Timemachine;
using timemachine::Range;
using timemachine::Data;
using timemachine::ID;
using timemachine::Empty;

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using grpc::ServerWriter;

using rocksdb::DBCloud;
using rocksdb::Options;

using DataWriter = ServerWriter<Data>;

namespace timemachine{

    class TSServer : public Timemachine::Service {

    public:
        TSServer();
        grpc::Status Save(ServerContext*, const Data*, Empty*) override;
        grpc::Status Get(ServerContext*, const ID*, Data*) override;
        grpc::Status GetRange(ServerContext*, const Range*, DataWriter*) override;
        grpc::Status Perform(std::string, std::function<grpc::Status (rocksdb::ColumnFamilyHandle*)>);
        void Init(std::shared_ptr<timemachine::DbClient>);
        
    private:
        std::shared_ptr<timemachine::DbClient> client;
        rocksdb::WriteOptions wopt;
        rocksdb::ReadOptions ropt;

    };
}




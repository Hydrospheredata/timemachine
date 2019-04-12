//
// Created by Dmitry Isaev on 2019-01-30.
//

#include "timeMachine.grpc.pb.h"
#include <functional>

#include "DbClient.h"
#include "utils/RepositoryUtils.h"

#ifndef TIMEMACHINE_GRPC_SERVER_H
#define TIMEMACHINE_GRPC_SERVER_H


using timemachine::Empty;
using timemachine::Timemachine;
using timemachine::RangeRequest;
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
using timemachine::utils::RepositoryUtils;

namespace timemachine {

    class GRPCServer : public Timemachine::Service {

    public:
        GRPCServer();

        grpc::Status Save(ServerContext *, const timemachine::SaveRequest*, ID *) override;

        grpc::Status Get(ServerContext *, const timemachine::GetRequest*, timemachine::Data*) override;

        grpc::Status GetRange(ServerContext *, const timemachine::RangeRequest*, DataWriter *) override;

        grpc::Status Perform(std::string, std::function<grpc::Status(rocksdb::ColumnFamilyHandle *)>);

        void Init(std::shared_ptr<timemachine::DbClient>);

    private:
        std::shared_ptr<timemachine::DbClient> client;
        rocksdb::ReadOptions ropt;

    };
}

#endif




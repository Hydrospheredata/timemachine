//
// Created by Dmitry Isaev on 2019-01-30.
//

#include "reqstore_service.grpc.pb.h"
#include <functional>

#include "DbClient.h"
#include "utils/RepositoryUtils.h"

#ifndef REQSTORE_GRPC_SERVER_H
#define REQSTORE_GRPC_SERVER_H


using hydrosphere::reqstore::Empty;
using hydrosphere::reqstore::Timemachine;
using hydrosphere::reqstore::RangeRequest;
using hydrosphere::reqstore::Data;
using hydrosphere::reqstore::ID;

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
using hydrosphere::reqstore::utils::RepositoryUtils;

namespace hydrosphere{
    namespace reqstore {

    class GRPCServer : public Timemachine::Service {

    public:
        GRPCServer();

        grpc::Status Save(ServerContext *, const hydrosphere::reqstore::SaveRequest*, ID *) override;

        grpc::Status Get(ServerContext *, const hydrosphere::reqstore::GetRequest*, hydrosphere::reqstore::Data*) override;

        grpc::Status GetRange(ServerContext *, const hydrosphere::reqstore::RangeRequest*, DataWriter *) override;

        grpc::Status GetSubsample(ServerContext*, const hydrosphere::reqstore::SubsampleRequest*, DataWriter*) override;

        grpc::Status Perform(std::string, std::function<grpc::Status(rocksdb::ColumnFamilyHandle *)>);

        grpc::Status PerformIfExists(std::string, std::function<grpc::Status(rocksdb::ColumnFamilyHandle *)>);

        void Init(std::shared_ptr<hydrosphere::reqstore::DbClient>);

    private:
        std::shared_ptr<hydrosphere::reqstore::DbClient> client;
        rocksdb::ReadOptions ropt;

    };
}
}

#endif




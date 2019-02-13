//
// Created by Dmitry Isaev on 2019-01-30.
//





// Created by Dmitry Isaev on 2019-01-30.
//

#include "timeMachine.grpc.pb.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include <functional>

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

        virtual ~TSServer();

        DBCloud* db;

        Status Save(ServerContext *context, const Data *request, Empty *response) override;

        Status Get(ServerContext *context, const ID *request, Data *response) override;

        Status GetRange(ServerContext *context, const Range *request, DataWriter *writer) override;

        Status Perform(std::string basename, std::function<Status (DBCloud&)> fn);


    };
}




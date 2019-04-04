#include <map>
#include <shared_mutex>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include "spdlog/spdlog.h"
#include "Config.h"
#include "IDComparator.h"
#include "timeMachine.grpc.pb.h"
#include "DbClient.h"

#ifndef TIMEMACHINE_CLOUD_DB_CLIENT_H
#define TIMEMACHINE_CLOUD_DB_CLIENT_H

namespace timemachine {

    class CloudDBClient : public DbClient {

    public:
        CloudDBClient(CloudDBClient &&);

        CloudDBClient(std::shared_ptr<Config>);

        virtual rocksdb::ColumnFamilyHandle* CreateColumnFamily(std::string &name) override;

    protected:
        virtual rocksdb::DB* getDB() override;

    private:
        rocksdb::CloudEnvOptions cloud_env_options;
        std::shared_ptr<rocksdb::CloudEnv> cloud_env;
        rocksdb::CloudEnv *cenv;
        rocksdb::DBCloud* db;
    };

}

#endif
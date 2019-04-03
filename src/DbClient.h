#include <map>
#include <shared_mutex>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include "spdlog/spdlog.h"
#include "Config.h"
#include "IDComparator.h"
#include "timeMachine.grpc.pb.h"

#ifndef TIMEMACHINE_DB_CLIENT_H
#define TIMEMACHINE_DB_CLIENT_H

namespace timemachine {

    class DbClient {

    public:
        DbClient(DbClient &&);

        DbClient(std::shared_ptr<Config>);

        timemachine::ID GenerateId();

        std::vector<rocksdb::ColumnFamilyDescriptor> GetColumnFamalies();

        std::shared_ptr<Config> cfg;

        rocksdb::ColumnFamilyHandle *GetColumnFamily(std::string &name);

        rocksdb::ColumnFamilyHandle *CreateColumnFamily(std::string &name);

        rocksdb::ColumnFamilyHandle *GetOrCreateColumnFamily(std::string &name);

        rocksdb::Status Put(const rocksdb::WriteOptions&, rocksdb::ColumnFamilyHandle*, const rocksdb::Slice&, const rocksdb::Slice&);

        rocksdb::Status Get(const rocksdb::ReadOptions&, rocksdb::ColumnFamilyHandle*, const rocksdb::Slice&, std::string*);

        void Iter(const rocksdb::ReadOptions&, rocksdb::ColumnFamilyHandle*, std::function<void(rocksdb::Iterator*)>);

    private:
        rocksdb::DBCloud *db;
        rocksdb::Options options;
        std::unordered_map<std::string, rocksdb::ColumnFamilyHandle *> columnFamalies;
        std::shared_timed_mutex lock;
        std::vector<rocksdb::ColumnFamilyHandle *> handles;
        rocksdb::CloudEnvOptions cloud_env_options;
        std::shared_ptr<rocksdb::CloudEnv> cloud_env;
        timemachine::IDComparator cmp;
        rocksdb::CloudEnv *cenv;
        std::string persistent_cache;
        std::atomic<long> uniqueGenerator;


    };

}

#endif

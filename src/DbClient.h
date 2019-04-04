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

        virtual timemachine::ID GenerateId();

        virtual std::vector<rocksdb::ColumnFamilyDescriptor> GetColumnFamalies();

        std::shared_ptr<Config> cfg;

        virtual rocksdb::ColumnFamilyHandle *GetColumnFamily(std::string &name);

        virtual rocksdb::ColumnFamilyHandle *CreateColumnFamily(std::string &name) = 0;

        virtual rocksdb::ColumnFamilyHandle *GetOrCreateColumnFamily(std::string &name);

        virtual rocksdb::Status Put(const rocksdb::WriteOptions&, rocksdb::ColumnFamilyHandle*, const rocksdb::Slice&, const rocksdb::Slice&);

        virtual rocksdb::Status Get(const rocksdb::ReadOptions&, rocksdb::ColumnFamilyHandle*, const rocksdb::Slice&, std::string*);

        virtual void Iter(const rocksdb::ReadOptions&, rocksdb::ColumnFamilyHandle*, std::function<void(rocksdb::Iterator*)>);

    protected:
    
        virtual rocksdb::DB* getDB() = 0;
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

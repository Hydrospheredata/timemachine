#include <map>
#include <shared_mutex>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include "spdlog/spdlog.h"
#include "Config.h"
#include "IDComparator.h"

#ifndef TIMEMACHINE_DB_CLIENT_H
#define TIMEMACHINE_DB_CLIENT_H

namespace timemachine {

    class DbClient {

    public:
        DbClient(DbClient &&);

        //DbClient(DbClient &);

        DbClient(std::shared_ptr<Config>);

        std::vector<rocksdb::ColumnFamilyDescriptor> GetColumnFamalies();

        rocksdb::DBCloud *db;
        std::shared_ptr<Config> cfg;

        rocksdb::ColumnFamilyHandle *GetColumnFamily(std::string &name);

        rocksdb::ColumnFamilyHandle *CreateColumnFamily(std::string &name);

        rocksdb::ColumnFamilyHandle *GetOrCreateColumnFamily(std::string &name);

    private:
        rocksdb::Options options;
        std::unordered_map<std::string, rocksdb::ColumnFamilyHandle *> columnFamalies;
        std::shared_timed_mutex lock;
        std::vector<rocksdb::ColumnFamilyHandle *> handles;
        rocksdb::CloudEnvOptions cloud_env_options;
        std::shared_ptr<rocksdb::CloudEnv> cloud_env;
        timemachine::IDComparator cmp;
        rocksdb::CloudEnv *cenv;
        std::string persistent_cache;

    };

}

#endif

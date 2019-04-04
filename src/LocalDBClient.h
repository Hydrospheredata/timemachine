#include <map>
#include <shared_mutex>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "spdlog/spdlog.h"
#include "Config.h"
#include "IDComparator.h"
#include "timeMachine.grpc.pb.h"
#include "DbClient.h"

#ifndef TIMEMACHINE_LOCAL_DB_CLIENT_H
#define TIMEMACHINE_LOCAL_DB_CLIENT_H

namespace timemachine {

    class LocalDBClient : public DbClient {

    public:
        LocalDBClient(LocalDBClient &&);

        LocalDBClient(std::shared_ptr<Config>);

        virtual rocksdb::ColumnFamilyHandle* CreateColumnFamily(std::string &name) override;

    protected:
        virtual rocksdb::DB* getDB() override;

    private:
        rocksdb::DB* db;
    };

}

#endif
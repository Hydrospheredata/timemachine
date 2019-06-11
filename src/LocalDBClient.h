#include <map>
#include <shared_mutex>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "spdlog/spdlog.h"
#include "Config.h"
#include "IDComparator.h"
#include "reqstore_service.grpc.pb.h"
#include "DbClient.h"

#ifndef REQSTORE_LOCAL_DB_CLIENT_H
#define REQSTORE_LOCAL_DB_CLIENT_H

namespace hydrosphere
{
namespace reqstore
{

class LocalDBClient : public DbClient
{

public:
    LocalDBClient(LocalDBClient &&);

    LocalDBClient(std::shared_ptr<Config>);

    virtual rocksdb::ColumnFamilyHandle *CreateColumnFamily(std::string &name) override;

protected:
    virtual rocksdb::DB *getDB() override;

private:
    rocksdb::DB *db;
};

} // namespace reqstore
} // namespace hydrosphere

#endif
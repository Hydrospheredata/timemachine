#include <map>
#include <shared_mutex>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include "spdlog/spdlog.h"
#include "Config.h"
#include "IDComparator.h"
#include "reqstore_service.grpc.pb.h"
#include "utils/RepositoryUtils.h"

#ifndef REQSTORE_DB_CLIENT_H
#define REQSTORE_DB_CLIENT_H

namespace hydrosphere
{
namespace reqstore
{

struct ColumnFamilyData
{
    ColumnFamilyData(ColumnFamilyData &);
    rocksdb::ColumnFamilyHandle *handle;
    std::atomic<long> lastUnique;
    ColumnFamilyData();
    ColumnFamilyData(ColumnFamilyData &&);
    ColumnFamilyData(rocksdb::ColumnFamilyHandle *_handle, long unsigned int _lastUnique);
};

struct UniqueRange
{
    unsigned long from;
    unsigned long till;
};

class DbClient : utils::RepositoryUtils
{

public:
    DbClient(DbClient &&);

    DbClient(std::shared_ptr<Config>);

    virtual hydrosphere::reqstore::ID GenerateId(std::string, unsigned long);

    virtual std::vector<rocksdb::ColumnFamilyDescriptor> GetColumnFamalies();

    std::shared_ptr<Config> cfg;

    std::vector<rocksdb::ColumnFamilyHandle *> handles;

    virtual rocksdb::ColumnFamilyHandle *GetColumnFamily(std::string &name);

    virtual rocksdb::ColumnFamilyHandle *CreateColumnFamily(std::string &name) = 0;

    virtual rocksdb::ColumnFamilyHandle *GetOrCreateColumnFamily(std::string &name);

    virtual rocksdb::Status Put(const rocksdb::WriteOptions &, rocksdb::ColumnFamilyHandle *, const rocksdb::Slice &, const rocksdb::Slice &);

    virtual rocksdb::Status Get(const rocksdb::ReadOptions &, rocksdb::ColumnFamilyHandle *, const rocksdb::Slice &, std::string *);

    virtual std::vector<rocksdb::Status> GetBatch(const rocksdb::ReadOptions &, rocksdb::ColumnFamilyHandle *, const std::vector<rocksdb::Slice> &, std::vector<std::string> *);

    virtual UniqueRange GetUniqueRange(rocksdb::ColumnFamilyHandle *);

    virtual UniqueRange GetUniqueRangeForTS(rocksdb::ColumnFamilyHandle *, unsigned long, unsigned long);

    virtual void Iter(const rocksdb::ReadOptions &, rocksdb::ColumnFamilyHandle *, const RangeRequest *, std::function<unsigned long int(hydrosphere::reqstore::ID, hydrosphere::reqstore::Data, bool)>);

protected:
    virtual rocksdb::DB *getDB() = 0;
    rocksdb::Options options;
    std::unordered_map<std::string, std::unique_ptr<ColumnFamilyData>> columnFamalies;
    std::shared_timed_mutex lock;
    rocksdb::CloudEnvOptions cloud_env_options;
    std::shared_ptr<rocksdb::CloudEnv> cloud_env;
    hydrosphere::reqstore::IDComparator cmp;
    rocksdb::CloudEnv *cenv;
    std::string persistent_cache;
};

} // namespace reqstore
} // namespace hydrosphere

#endif

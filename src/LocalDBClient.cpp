#include "Config.h"
#include <iostream>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/options.h"
#include "Config.h"
#include "spdlog/spdlog.h"
#include "DbClient.h"
#include "CloudDBClient.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include "LocalDBClient.h"

namespace hydrosphere
{
namespace reqstore
{

using MutexType = std::shared_timed_mutex;
using ReadLock = std::shared_lock<MutexType>;
using WriteLock = std::unique_lock<MutexType>;

LocalDBClient::LocalDBClient(LocalDBClient &&other) : DbClient(std::move(other))
{
    spdlog::info("CloudDBClient move constructor");
    db = std::move(other.db);
}

LocalDBClient::LocalDBClient(std::shared_ptr<Config> _cfg) : DbClient(_cfg)
{
    spdlog::info("opening db with name: {}", cfg->dbName);
    spdlog::info("                persistent_cache: {}", persistent_cache);

    // Strange thing rocksdb::DB::ListColumnFamilies doesn't work if you haven't open db yet
    // but Open func returns InvalidArguments (you should open all existing column famalies)
    // WTF???
    rocksdb::DB *_fakeDb;

    rocksdb::Status fakeStatus = rocksdb::DB::Open(options, cfg->sourceLocalDir, &_fakeDb);

    auto cfDescriptors = GetColumnFamalies();
    spdlog::debug("Column family descriptors fetched, size: {}", cfDescriptors.size());

    delete _fakeDb;

    rocksdb::Status dbStatus = rocksdb::DB::Open(options, cfg->sourceLocalDir, cfDescriptors, &handles, &db);

    if (handles.size() > 0)
    {
        spdlog::debug("Adding handles to columnFamalies cache. handles size is: {}", handles.size());
        for (auto it = handles.begin(); it != handles.end(); ++it)
        {
            auto cfHandle = *it;
            spdlog::debug("Adding {}", cfHandle->GetName());
            auto lastUnique = LastUnique(cfHandle);
            columnFamalies[cfHandle->GetName()] = ColumnFamilyData(cfHandle, lastUnique);
        }
    }

    if (!dbStatus.ok())
    {
        spdlog::error("Unable to open db: {0}", dbStatus.ToString());
        throw std::runtime_error("Failed to connect to DB");
    }
    spdlog::info("Rocksdb-cloud local connection opened: {0}", dbStatus.ToString());
}

rocksdb::DB *LocalDBClient::getDB()
{
    return db;
}

rocksdb::ColumnFamilyHandle *LocalDBClient::CreateColumnFamily(std::string &name)
{
    std::lock_guard<std::shared_timed_mutex> writerLock(lock);
    spdlog::debug("trying to create columnFamily by name: {}", name);
    spdlog::debug("db with name {} is here!!!!", getDB()->GetName());
    rocksdb::ColumnFamilyHandle *cf;

    rocksdb::ColumnFamilyOptions cfOptions;
    hydrosphere::reqstore::IDComparator cmp;
    cfOptions.comparator = &cmp;

    rocksdb::Status s = getDB()->CreateColumnFamily(cfOptions, name, &cf);
    if (s.OK)
    {
        spdlog::info("trying to cache ColumnFamilyHandler with name: {}", name, cf->GetName());
        auto cfDescriptors = GetColumnFamalies();

        delete db;

        rocksdb::Status dbStatus = rocksdb::DB::Open(options, cfg->sourceLocalDir, cfDescriptors, &handles, &db);
        if (handles.size() > 0)
        {
            spdlog::debug("Adding handles to columnFamalies cache. handles size is: {}", handles.size());
            for (auto it = handles.begin(); it != handles.end(); ++it)
            {
                auto cfHandle = *it;
                spdlog::debug("Adding {}", cfHandle->GetName());
                auto lastUnique = LastUnique(cfHandle);
                columnFamalies[cfHandle->GetName()] = ColumnFamilyData(cfHandle, lastUnique);
            }
        }

        if (!dbStatus.ok())
        {
            spdlog::error("Unable to open db: {0}", dbStatus.ToString());
            throw std::runtime_error("Failed to connect to DB");
        }
        spdlog::info("Rocksdb-cloud connection opened: {0}", dbStatus.ToString());
        return cf;
    }

    throw std::runtime_error("Couldn't create columnFamily: " + name);
};

} // namespace reqstore
} // namespace hydrosphere

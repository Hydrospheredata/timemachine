#include "Config.h"
#include <iostream>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include "Config.h"
#include "spdlog/spdlog.h"
#include "DbClient.h"
#include "CloudDBClient.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"

namespace hydrosphere
{
namespace reqstore
{

using MutexType = std::shared_timed_mutex;
using ReadLock = std::shared_lock<MutexType>;
using WriteLock = std::unique_lock<MutexType>;

CloudDBClient::CloudDBClient(CloudDBClient &&other) : DbClient(std::move(other))
{
    spdlog::info("CloudDBClient move constructor");
    cloud_env = std::move(other.cloud_env);
    cloud_env_options = std::move(other.cloud_env_options);
    cenv = std::move(other.cenv);
    db = std::move(other.db);
}

CloudDBClient::CloudDBClient(std::shared_ptr<Config> _cfg) : DbClient(_cfg)
{
    spdlog::info("CloudDBClient::CloudDBClient");
    cloud_env_options.credentials.access_key_id.assign(cfg->keyid);
    cloud_env_options.credentials.secret_key.assign(cfg->secret);
    if (cfg->useKinesis)
    {
        cloud_env_options.keep_local_log_files = false;
        cloud_env_options.log_type = rocksdb::LogType::kLogKinesis;
    }

    rocksdb::Status s =
        rocksdb::CloudEnv::NewAwsEnv(rocksdb::Env::Default(),
                                     cfg->sourceBucket, cfg->sourceLocalDir, cfg->kRegion,
                                     cfg->destBucket, cfg->destinationLocalDir, cfg->kRegion,
                                     cloud_env_options, options.info_log, &cenv);

    if (!s.ok())
    {
        spdlog::error("Unable to create cloud env in bucket {0}. {1}", cfg->sourceBucket, s.ToString());
        throw std::runtime_error("Failed to connect NewAwsEnv");
    }

    cloud_env.reset(cenv);

    options.env = cloud_env.get();
    spdlog::info("AWSEnv initialized: {}", s.ToString());

    persistent_cache = "";

    spdlog::info("opening db with name: {}", cfg->dbName);
    spdlog::info("                persistent_cache: {}", persistent_cache);

    // Strange thing rocksdb::DB::ListColumnFamilies doesn't work if you haven't open db yet
    // but Open func returns InvalidArguments (you should open all existing column famalies)
    // WTF???
    rocksdb::DBCloud *_fakeDb;
    rocksdb::Status fakeStatus = rocksdb::DBCloud::Open(options, cfg->dbName, persistent_cache, 0, &_fakeDb);
    auto cfDescriptors = GetColumnFamalies();
    spdlog::debug("Column family descriptors fetched, size: {}", cfDescriptors.size());

    rocksdb::Status dbStatus = rocksdb::DBCloud::Open(options, cfg->dbName, cfDescriptors, persistent_cache, 0,
                                                      &handles, &db);

    if (handles.size() > 0)
    {
        spdlog::debug("Adding handles to columnFamalies cache. handles size is: {}", handles.size());
        for (auto it = handles.begin(); it != handles.end(); ++it)
        {
            auto cfHandle = *it;
            spdlog::debug("Adding {}", cfHandle->GetName());
            auto lastUnique = GetUniqueRange(cfHandle).till;
            columnFamalies[cfHandle->GetName()] = std::unique_ptr<ColumnFamilyData>(new ColumnFamilyData(cfHandle, lastUnique));
        }
    }

    if (!dbStatus.ok())
    {
        spdlog::error("Unable to open db at path {0} with bucket {1}. {2}", cfg->dbName, cfg->sourceBucket,
                      dbStatus.ToString());
        throw std::runtime_error("Failed to connect to DB");
    }
    spdlog::info("Rocksdb-cloud connection opened: {0}", dbStatus.ToString());
}

rocksdb::DB *CloudDBClient::getDB()
{
    return db;
}

rocksdb::ColumnFamilyHandle *CloudDBClient::CreateColumnFamily(std::string &name)
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

        rocksdb::Status dbStatus = rocksdb::DBCloud::Open(options, cfg->dbName, cfDescriptors, persistent_cache, 0,
                                                          &handles, &db);

        if (handles.size() > 0)
        {
            spdlog::debug("Adding handles to columnFamalies cache. handles size is: {}", handles.size());
            for (auto it = handles.begin(); it != handles.end(); ++it)
            {
                auto cfHandle = *it;
                spdlog::debug("Adding {}", cfHandle->GetName());
                auto lastUnique = GetUniqueRange(cfHandle).till;
                columnFamalies[cfHandle->GetName()] = std::unique_ptr<ColumnFamilyData>(new ColumnFamilyData(cfHandle, lastUnique));
            }
        }

        if (!dbStatus.ok())
        {
            spdlog::error("Unable to open db at path {0} with bucket {1}. {2}", cfg->dbName, cfg->sourceBucket,
                          dbStatus.ToString());
            throw std::runtime_error("Failed to connect to DB");
        }
        spdlog::info("Rocksdb-cloud connection opened: {0}", dbStatus.ToString());

        return cf;
    }

    throw std::runtime_error("Couldn't create columnFamily: " + name);
};

} // namespace reqstore
} // namespace hydrosphere

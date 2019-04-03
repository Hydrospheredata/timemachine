#include "Config.h"
#include <iostream>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include "Config.h"
#include "spdlog/spdlog.h"
#include "DbClient.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"


namespace timemachine {

    using MutexType = std::shared_timed_mutex;
    using ReadLock  = std::shared_lock<MutexType>;
    using WriteLock = std::unique_lock<MutexType>;

    DbClient::DbClient(DbClient &&other):uniqueGenerator(0) {
        spdlog::info("DbClient move constructor");
        WriteLock rhs_lock(other.lock);
        cloud_env = std::move(other.cloud_env);
        options = std::move(other.options);
        cfg = std::move(other.cfg);
        handles = std::move(other.handles);
        columnFamalies = std::move(other.columnFamalies);
        db = std::move(other.db);
        cmp = std::move(other.cmp);
    }

    timemachine::ID DbClient::GenerateId() {
        auto ms = std::chrono::system_clock::now().time_since_epoch().count();

        timemachine::ID id;

        id.set_timestamp(ms);
        unsigned long int unique = uniqueGenerator.fetch_add(1, std::memory_order_release);
        id.set_unique(unique);

        return id;
    }

    rocksdb::Status DbClient::Get(const rocksdb::ReadOptions& options, rocksdb::ColumnFamilyHandle* handle, const rocksdb::Slice& key, std::string* data){
        std::shared_lock<std::shared_timed_mutex> readLock(lock);
        return db->Get(options, handle, key, data);
    }

    rocksdb::Status DbClient::Put(const rocksdb::WriteOptions& wopt, rocksdb::ColumnFamilyHandle* handle, const rocksdb::Slice& key, const rocksdb::Slice& val){
        std::shared_lock<std::shared_timed_mutex> readLock(lock);
        return db->Put(wopt, handle, key, val);
    }

    void DbClient::Iter(const rocksdb::ReadOptions& ropt, rocksdb::ColumnFamilyHandle* handle, std::function<void(rocksdb::Iterator*)> fn){
        std::shared_lock<std::shared_timed_mutex> readLock(lock);
        auto iter = db->NewIterator(ropt, handle);
        fn(iter);
    }

    DbClient::DbClient(std::shared_ptr<Config> _cfg): uniqueGenerator(0) {
        cfg = _cfg;
        options.comparator = &cmp;
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        options.create_if_missing = true;

        rocksdb::Env *base_env_ = rocksdb::Env::Default();
        base_env_->NewLogger("./rocksdb-cloud.log", &options.info_log);
        cloud_env_options.credentials.access_key_id.assign(cfg->keyid);
        cloud_env_options.credentials.secret_key.assign(cfg->secret);
        if (cfg->useKinesis) {
            cloud_env_options.keep_local_log_files = false;
            cloud_env_options.log_type = rocksdb::LogType::kLogKinesis;
        }

        rocksdb::Status s =
                rocksdb::CloudEnv::NewAwsEnv(rocksdb::Env::Default(),
                                             cfg->sourceBucket, cfg->sourceLocalDir, cfg->kRegion,
                                             cfg->destBucket, cfg->destinationLocalDir, cfg->kRegion,
                                             cloud_env_options, options.info_log, &cenv);

        if (!s.ok()) {
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

        if (handles.size() > 0) {
            spdlog::debug("Adding handles to columnFamalies cache. handles size is: {}", handles.size());
            for (auto it = handles.begin(); it != handles.end(); ++it) {
                auto cfHandle = *it;
                spdlog::debug("Adding {}", cfHandle->GetName());
                columnFamalies[cfHandle->GetName()] = cfHandle;
            }
        }

        if (!dbStatus.ok()) {
            spdlog::error("Unable to open db at path {0} with bucket {1}. {2}", cfg->dbName, cfg->sourceBucket,
                          dbStatus.ToString());
            throw std::runtime_error("Failed to connect to DB");
        }
        spdlog::info("Rocksdb-cloud connection opened: {0}", dbStatus.ToString());
    }

    std::vector<rocksdb::ColumnFamilyDescriptor> DbClient::GetColumnFamalies() {
        spdlog::debug("trying to fetch columnFamily names list by dbName: {0}", cfg->ToString());
        std::vector<std::string> cf_names;
        rocksdb::Status s = rocksdb::DB::ListColumnFamilies(options, cfg->dbName, &cf_names);

        if (!s.ok()) {
            spdlog::info("Unable to return ColumnFamaliesList: {0}", s.code());
            throw std::runtime_error("Unable to return ColumnFamaliesList");
        }

        std::vector<rocksdb::ColumnFamilyDescriptor> descriptors;

        for (auto &&cfName: cf_names) {
            spdlog::info("\n\t -{}", cfName);
            //TODO: Not necessary to allocate new
            rocksdb::ColumnFamilyOptions *cfOptions = new rocksdb::ColumnFamilyOptions;
            //TODO: Comparator could be shared
            timemachine::IDComparator *cmp = new timemachine::IDComparator;
            cfOptions->comparator = cmp;
            descriptors.push_back(rocksdb::ColumnFamilyDescriptor(cfName, *cfOptions));
        }
        return descriptors;
    }


    rocksdb::ColumnFamilyHandle *DbClient::GetColumnFamily(std::string &name) {
        std::shared_lock<std::shared_timed_mutex> readLock(lock);
        spdlog::debug("trying to find columnFamily by name: {}", name);
        auto pos = columnFamalies.find(name);
        if (pos == columnFamalies.end()) {
            spdlog::debug("columnFamily {} doesn't exist", name);
            return nullptr;
        } else {
            spdlog::debug("columnFamily {} been found", name);
            return pos->second;
        }
    };

    rocksdb::ColumnFamilyHandle *DbClient::CreateColumnFamily(std::string &name) {
        std::lock_guard<std::shared_timed_mutex> writerLock(lock);
        spdlog::debug("trying to create columnFamily by name: {}", name);
        spdlog::debug("db with name {} is here!!!!", db->GetName());
        rocksdb::ColumnFamilyHandle *cf;

        rocksdb::ColumnFamilyOptions cfOptions;
        timemachine::IDComparator cmp;
        cfOptions.comparator = &cmp;

        rocksdb::Status s = db->CreateColumnFamily(cfOptions, name, &cf);
        if (s.OK) {
            spdlog::info("trying to cache ColumnFamilyHandler with name: {}", name, cf->GetName());
            auto cfDescriptors = GetColumnFamalies();

            for(auto it = handles.begin(); it != handles.end(); ++it){
                delete *it;
            }

            delete db;

            rocksdb::Status dbStatus = rocksdb::DBCloud::Open(options, cfg->dbName, cfDescriptors, persistent_cache, 0,
                                                              &handles, &db);

            if (handles.size() > 0) {
                spdlog::debug("Adding handles to columnFamalies cache. handles size is: {}", handles.size());
                for (auto it = handles.begin(); it != handles.end(); ++it) {
                    auto cfHandle = *it;
                    spdlog::debug("Adding {}", cfHandle->GetName());
                    columnFamalies[cfHandle->GetName()] = cfHandle;
                }
            }

            if (!dbStatus.ok()) {
                spdlog::error("Unable to open db at path {0} with bucket {1}. {2}", cfg->dbName, cfg->sourceBucket,
                              dbStatus.ToString());
                throw std::runtime_error("Failed to connect to DB");
            }
            spdlog::info("Rocksdb-cloud connection opened: {0}", dbStatus.ToString());

            return cf;
        }

        throw std::runtime_error("Couldn't create columnFamily: " + name);

    };

    rocksdb::ColumnFamilyHandle *DbClient::GetOrCreateColumnFamily(std::string &name) {
        auto exists = GetColumnFamily(name);
        if (exists) return exists;

        auto created = CreateColumnFamily(name);
        return created;
    };

} 

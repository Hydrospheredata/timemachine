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
        return getDB()->Get(options, handle, key, data);
    }

    rocksdb::Status DbClient::Put(const rocksdb::WriteOptions& wopt, rocksdb::ColumnFamilyHandle* handle, const rocksdb::Slice& key, const rocksdb::Slice& val){
        std::shared_lock<std::shared_timed_mutex> readLock(lock);
        return getDB()->Put(wopt, handle, key, val);
    }

    // void DbClient::Iter(const rocksdb::ReadOptions& ropt, rocksdb::ColumnFamilyHandle* handle, std::function<void(rocksdb::Iterator*)> fn){
    //     std::shared_lock<std::shared_timed_mutex> readLock(lock);
    //     auto iter = getDB()->NewIterator(ropt, handle);
    //     fn(iter);
    // }

    void DbClient::Iter(const rocksdb::ReadOptions& ropt, rocksdb::ColumnFamilyHandle* handle, const RangeRequest *request, std::function<unsigned long int(timemachine::ID, timemachine::Data, bool)> fn){
        std::shared_lock<std::shared_timed_mutex> readLock(lock);
        auto iter = getDB()->NewIterator(ropt, handle);

        if (!request->reverse() && request->from() == 0) {
            spdlog::debug("seek from start");
            iter->SeekToFirst();
        } else if(request->reverse() && request->till() == 0){
            spdlog::debug("reverse seek from end");
            iter->SeekToLast();
        } else {

            timemachine::ID keyFrom;
            if(!request->reverse()){
                keyFrom.set_timestamp(request->from());
            } else {
                keyFrom.set_timestamp(request->till());
            }
            
            char bytes[16];
            SerializeID(&keyFrom, bytes);
            auto keyFromSlice = rocksdb::Slice(bytes, 16);
            spdlog::debug("seek from {}", request->from());

            if(!request->reverse()){
                iter->SeekForPrev(keyFromSlice);
            } else {
                iter->SeekForPrev(keyFromSlice);
            }
            
        }

        bool stopIteration = false;

        unsigned long int bytesSend = 0;
        unsigned long int messagesSend = 0;

        for (; iter->Valid(); request->reverse() ? iter->Prev() : iter->Next()) {

            if(stopIteration) break;

            auto keyString = iter->key();
            auto folder = request->folder();
            auto key = RepositoryUtils::DeserializeID(keyString);
            if (!request->reverse(), request->till()!= 0 && request->till() < key.timestamp()) break;
            if (request->reverse(), request->from()!= 0 && request->from() >= key.timestamp()) break;
            if (!request->reverse(), request->from() > key.timestamp()) continue;
            if (request->maxbytes() != 0 && request->maxbytes() <= bytesSend) break;
            if (request->maxmessages() != 0 && request->maxmessages() <= messagesSend) break;

            auto val = iter->value().ToString();
            auto data = Data();
            data.ParseFromString(val);

            spdlog::debug("iterating key ({0}, {1}) from folder {2}",
                data.id().timestamp(),
                data.id().unique(),
                request->folder()
            );

            unsigned long int sent = fn(key, data, stopIteration);
            messagesSend += 1;
            bytesSend += sent;

        }
        delete iter;
        spdlog::debug("return {0} status", "OK");  
    }

    DbClient::DbClient(std::shared_ptr<Config> _cfg): uniqueGenerator(0) {
        spdlog::info("DbClient::DbClient");
        cfg = _cfg;
        options.comparator = &cmp;
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        options.create_if_missing = true;
        options.create_missing_column_families = true;

        rocksdb::Env *base_env_ = rocksdb::Env::Default();
        base_env_->NewLogger("./rocksdb-cloud.log", &options.info_log);
    }

    std::vector<rocksdb::ColumnFamilyDescriptor> DbClient::GetColumnFamalies() {
        spdlog::debug("trying to fetch columnFamily names list by dbName: {0}", cfg->ToString());
        std::vector<std::string> cf_names;
        rocksdb::Status s = rocksdb::DB::ListColumnFamilies(options, cfg->dbName, &cf_names);

        if (!s.ok()) {
            spdlog::info("Unable to return ColumnFamaliesList: {0}, {1}", s.code(), s.ToString());
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

    rocksdb::ColumnFamilyHandle *DbClient::GetOrCreateColumnFamily(std::string &name) {
        auto exists = GetColumnFamily(name);
        if (exists) return exists;

        auto created = CreateColumnFamily(name);
        return created;
    };

} 

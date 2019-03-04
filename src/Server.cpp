//
// Created by Dmitry Isaev on 2019-01-30.
//

#include "Server.h"
#include <functional>
#include "spdlog/spdlog.h"
#include <shared_mutex>

using namespace rocksdb;

namespace timemachine {

    TSServer::TSServer(){
        wopt = WriteOptions();
        wopt.disableWAL = false;

        ropt = ReadOptions();
    }

    grpc::Status TSServer::Perform(std::string baseName, std::function<grpc::Status(rocksdb::ColumnFamilyHandle*)> fn) {

        try {
            auto cf = client->GetOrCreateColumnFamily(baseName);
            auto result = fn(cf);
            return result;
        } catch (const std::exception &e) {
            return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong: " + std::string(e.what()));
        } catch (...) {
            return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong");
        }
    }

    void TSServer::Init(std::shared_ptr<timemachine::DbClient> _db){
        client = _db;
    }

    grpc::Status TSServer::Save(ServerContext *context, const Data *request, Empty *response) {

        std::function<grpc::Status(ColumnFamilyHandle* cf)> action = [&](ColumnFamilyHandle* cf) -> grpc::Status {
            try {
                spdlog::debug("Saving data with key ({0}, {1}, {2})", 
                    request->id().timestamp(), 
                    request->id().incremental(),
                    request->id().folder()
                );

                rocksdb::Status putStatus = client->db->Put(wopt, cf, request->id().SerializeAsString(), request->SerializeAsString());
                spdlog::debug("PUT status is {0}", putStatus.ToString());

                if (!putStatus.ok()) {
                    spdlog::error("PUT status is {0}", putStatus.ToString());
                    return grpc::Status(grpc::StatusCode::INTERNAL, "Something wrong :( " + putStatus.ToString());
                }
                spdlog::debug("Saved data  with key ({0}, {1}, {2})", 
                    request->id().timestamp(),
                    request->id().incremental(),
                    request->id().folder()
                );
                return grpc::Status::OK;
            } catch (const std::exception &e) {
                return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong: " + std::string(e.what()));
            } catch (...) {
                return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong");
            }
        };

        grpc::Status status = Perform(request->id().folder(), action);

        return status;
    }

    grpc::Status TSServer::Get(ServerContext *context, const ID *request, Data *response) {
        auto status = Perform(request->folder(), [&](ColumnFamilyHandle* cf) -> grpc::Status {
            try {
                client->db->Savepoint();
                spdlog::debug("Get data by key ({0}, {1}, {2})", 
                request->timestamp(), 
                request->incremental(),
                request->folder());

                std::string data;
                rocksdb::Status getStatus = client->db->Get(ropt, cf, request->SerializeAsString(), &data);
                spdlog::debug("GET status is  {0}", getStatus.ToString());
                if (!getStatus.ok()) {
                    spdlog::error("GET status is  {0}", getStatus.ToString());
                    return grpc::Status(grpc::StatusCode::INTERNAL, "Something wrong :( " + getStatus.ToString());
                }
                response->ParseFromString(data);
                return grpc::Status::OK;
            } catch (const std::exception &e) {
                return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong: " + std::string(e.what()));
            } catch (...) {
                return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong");
            }
        });

        return status;
    }

    grpc::Status TSServer::GetRange(ServerContext *context, const Range *request, DataWriter *writer) {
        auto status = Perform(request->folder(), [&](ColumnFamilyHandle* cf) -> grpc::Status {
            try {
                spdlog::debug("Get range from {0} to {1}", request->from().timestamp(), request->till().timestamp());
                std::string key;
                request->SerializePartialToString(&key);
                std::string data;

                spdlog::debug("getting iterator from {0} to {1}", request->from().timestamp(), request->till().timestamp());

                Iterator *it = client->db->NewIterator(ropt, cf);

                if (request->from().timestamp() == 0) {
                    spdlog::debug("seek from start");
                    it->SeekToFirst();
                } else {
                    spdlog::debug("seek from {}", request->from().timestamp());
                    it->SeekForPrev(request->from().SerializeAsString());
                }

                timemachine::IDComparator comparator;

                for (; it->Valid(); it->Next()) {

                    if(context->IsCancelled()) break;

                    auto keyString = it->key().ToString();
                    auto key = ID();
                    key.ParseFromString(keyString);

                    auto slice = rocksdb::Slice(request->from().SerializeAsString());
                    spdlog::debug("request->till().timestamp() is {}", request->till().timestamp());
                    if(request->till().timestamp() != 0 && comparator.Compare(slice, it->key()) < 0) break;

                    spdlog::debug("iterating key ({0}, {1}, {2})", 
                        key.timestamp(),
                        key.incremental(),
                        key.folder()
                    );

                    auto val = it->value().ToString();
                    auto data = Data();
                    data.ParseFromString(val);
                    writer->Write(data);
                }

                delete it;

                spdlog::debug("return {0} status", "OK");

                return grpc::Status::OK;
            } catch (const std::exception &e) {
                return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong: " + std::string(e.what()));
            } catch (...) {
                return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong");
            }
        });

        return status;
    }

}


//
// Created by Dmitry Isaev on 2019-01-30.
//

#include "GRPCServer.h"
#include <functional>
#include "spdlog/spdlog.h"
#include <shared_mutex>

using namespace rocksdb;

namespace timemachine {

    GRPCServer::GRPCServer() : timemachine::utils::RepositoryUtils() {
        wopt = WriteOptions();
        wopt.disableWAL = false;

        ropt = ReadOptions();
    }

    grpc::Status
    GRPCServer::Perform(std::string baseName, std::function<grpc::Status(rocksdb::ColumnFamilyHandle *)> fn) {

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

    void GRPCServer::Init(std::shared_ptr<timemachine::DbClient> _db) {
        client = _db;
    }

    grpc::Status GRPCServer::Save(ServerContext *context, const Data *request, ID *response) {

        std::function<grpc::Status(ColumnFamilyHandle *cf)> action = [&](ColumnFamilyHandle *cf) -> grpc::Status {

            auto id = GenerateId(request->id().folder());
            timemachine::Data data;

            timemachine::ID* id_ptr = &id;
            data.set_data(request->data());
            data.mutable_id()->set_timestamp(id.timestamp());
            data.mutable_id()->set_unique(id.unique());
            //TODO: Do you really need folder field?
            data.mutable_id()->set_folder(id.folder());

            rocksdb::Status putStatus = client->db->Put(wopt, cf, id.SerializeAsString(),
                                                        data.SerializeAsString());
            spdlog::debug("PUT status is {0}", putStatus.ToString());

            if (!putStatus.ok()) {
                spdlog::error("PUT status is {0}", putStatus.ToString());

                return grpc::Status(grpc::StatusCode::INTERNAL, "Something wrong :( " + putStatus.ToString());
            }
            spdlog::debug("Saved data  with key ({0}, {1}, {2})",
                          id.timestamp(),
                          id.unique(),
                          id.folder()
            );
            return grpc::Status::OK;
        };

        grpc::Status status = Perform(request->id().folder(), action);

        return status;
    }

    grpc::Status GRPCServer::Get(ServerContext *context, const ID *request, Data *response) {
        auto status = Perform(request->folder(), [&](ColumnFamilyHandle *cf) -> grpc::Status {

            client->db->Savepoint();
            spdlog::debug("Get data by key ({0}, {1}, {2})",
                          request->timestamp(),
                          request->unique(),
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
        });

        return status;
    }

    grpc::Status GRPCServer::GetRange(ServerContext *context, const Range *request, DataWriter *writer) {
        auto status = Perform(request->folder(), [&](ColumnFamilyHandle *cf) -> grpc::Status {
            spdlog::debug("Get range from {0} to {1}", request->from().timestamp(), request->till().timestamp());

            std::string key;
            std::string data;

            //request->SerializePartialToString(&key);

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

                if (context->IsCancelled()) break;

                auto keyString = it->key().ToString();
                auto key = ID();
                key.ParseFromString(keyString);

                auto slice = rocksdb::Slice(request->from().SerializeAsString());
                spdlog::debug("request->till().timestamp() is {}", request->till().timestamp());
                // TODO: No need for comparator...
                if (request->till().timestamp() != 0 && comparator.Compare(slice, it->key()) < 0) break;

                spdlog::debug("iterating key ({0}, {1}, {2})",
                              key.timestamp(),
                              key.unique(),
                              key.folder()
                );

                //TODO: Maybe better not a string???
                auto val = it->value().ToString();
                auto data = Data();
                data.ParseFromString(val);
                writer->Write(data);
            }

            delete it;

            spdlog::debug("return {0} status", "OK");

            return grpc::Status::OK;
        });

        return status;
    }

}


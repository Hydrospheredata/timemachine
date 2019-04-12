//
// Created by Dmitry Isaev on 2019-01-30.
//

#include "GRPCServer.h"
#include <functional>
#include "spdlog/spdlog.h"
#include <shared_mutex>

using namespace rocksdb;

namespace timemachine {

    GRPCServer::GRPCServer() {
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

    grpc::Status GRPCServer::Save(ServerContext *context, const SaveRequest *request, ID *response) {

        std::function<grpc::Status(ColumnFamilyHandle *cf)> action = [&](ColumnFamilyHandle *cf) -> grpc::Status {

            auto id = client->GenerateId();
            timemachine::Data data;

            timemachine::ID* id_ptr = &id;
            data.set_data(request->data());
            data.mutable_id()->set_timestamp(id.timestamp());
            data.mutable_id()->set_unique(id.unique());
            char bytes[16];
            RepositoryUtils::SerializeID(id_ptr, bytes);
            auto bynaryId = rocksdb::Slice(bytes, 16);

            rocksdb::WriteOptions wopt;
            wopt.disableWAL = !request->usewal();

            rocksdb::Status putStatus = client->Put(wopt, cf, bynaryId, data.SerializeAsString());
            spdlog::debug("PUT status is {0}", putStatus.ToString());

            if (!putStatus.ok()) {
                spdlog::error("PUT status is {0}", putStatus.ToString());

                return grpc::Status(grpc::StatusCode::INTERNAL, "Something wrong :( " + putStatus.ToString());
            }
            spdlog::debug("Saved data  with key ({0}, {1}) to folder {2}",
                          data.id().timestamp(),
                          data.id().unique(),
                          request->folder()
            );

            response->set_timestamp(id.timestamp());
            response->set_unique(id.unique());
            return grpc::Status::OK;
        };

        grpc::Status status = Perform(request->folder(), action);

        return status;
    }

    grpc::Status GRPCServer::Get(ServerContext *context, const timemachine::GetRequest *request, Data *response) {
        auto status = Perform(request->folder(), [&](ColumnFamilyHandle *cf) -> grpc::Status {

            spdlog::debug("Get data by key ({0}, {1}) from folder {2}",
                          request->timestamp(),
                          request->unique(),
                          request->folder());

            std::string data;
            char bytes[16];
            timemachine::ID req_id;
            req_id.set_timestamp(request->timestamp());
            req_id.set_unique(request->unique());
            RepositoryUtils::SerializeID(&req_id, bytes);
            auto bynaryId = rocksdb::Slice(bytes, 16);
            rocksdb::Status getStatus = client->Get(ropt, cf, bynaryId, &data);
            spdlog::debug("GET status is  {0}", getStatus.ToString());
            if (!getStatus.ok()) {
                spdlog::error("GET status is  {0}", getStatus.ToString());
                return grpc::Status(grpc::StatusCode::INTERNAL, "Something wrong :( " + getStatus.ToString());
            }
            timemachine::Data result;
            result.set_data(data);
            result.mutable_id()->set_timestamp(request->timestamp());
            result.mutable_id()->set_unique(request->unique());
            response->ParseFromString(data);
            return grpc::Status::OK;
        });

        return status;
    }

    grpc::Status GRPCServer::GetRange(ServerContext *context, const RangeRequest *request, DataWriter *writer) {
        auto status = Perform(request->folder(), [&](ColumnFamilyHandle *cf) -> grpc::Status {
            spdlog::debug("Get range from {0} to {1}", request->from(), request->till());

            std::string data;

            spdlog::debug("getting iterator from {0} to {1}", request->from(), request->till());

            client->Iter(ropt, cf, request, [&](timemachine::ID key, timemachine::Data data, bool stop) -> unsigned long int {
                if (context->IsCancelled()) stop = true;
                writer->Write(data);
                return data.SerializeAsString().size();
            });

            return grpc::Status::OK;
        });

        return status;
    }

}


//
// Created by Dmitry Isaev on 2019-01-30.
//

#include "Server.h"
#include "rocksdb/options.h"
#include "rocksdb/cloud/db_cloud.h"
#include <functional>

using namespace rocksdb;

namespace timemachine {


    grpc::Status TSServer::Perform(std::string baseName, std::function<grpc::Status(DBCloud&)> fn) {

        try {
            auto result = fn(*db);
            return result;
        } catch (const std::exception &e) {
            return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong: " + std::string(e.what()));
        } catch (...) {
            return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong");
        }
    }

    grpc::Status TSServer::Save(ServerContext *context, const Data *request, Empty *response) {

        std::function<grpc::Status(DBCloud&)> action = [&](DBCloud& db) -> grpc::Status {
            try {
                std::cout << "Saving data with key  " << request->id().uuid() << std::endl;

                // options for each write
                WriteOptions wopt;
                wopt.disableWAL = false;

                rocksdb::Status putStatus = db.Put(wopt, request->id().uuid(), request->SerializeAsString());
                std::cout << "PUT status is " << putStatus.ToString() << std::endl;
                if (!putStatus.ok()) {
                    std::cerr << "PUT status is " << putStatus.ToString() << std::endl;
                    return grpc::Status(grpc::StatusCode::INTERNAL, "Something wrong :( " + putStatus.ToString());
                }

                std::cout << "Saved data  with key  " << request->id().uuid() << std::endl;
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
        auto status = Perform(request->folder(), [&](DBCloud& db) -> grpc::Status {
            try {
                std::cout << "Get data by key" << request->uuid() << std::endl;
                std::string data;
                rocksdb::Status getStatus = db.Get(ReadOptions(), request->uuid(), &data);
                std::cout << "GET status is " << getStatus.ToString() << std::endl;
                if (!getStatus.ok()) {
                    std::cerr << "GET status is " << getStatus.ToString() << std::endl;
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
        auto status = Perform(request->folder(), [&](DBCloud& db) -> grpc::Status {
            try {
                std::cout << "Get range " << request << std::endl;
                std::string key;
                request->SerializePartialToString(&key);
                std::string data;

                std::cout << "getting iterator " << request << std::endl;

                Iterator *it = db.NewIterator(ReadOptions());

                if (request->from().empty()) {
                    it->SeekToFirst();
                } else {
                    it->Seek(request->from());
                }

                for (; it->Valid(); it->Next()) {
                    auto keyString = it->key().ToString();

                    if (keyString < request->from()) break;

                    std::cout << "Reading key " << keyString << std::endl;

                    auto val = it->value().ToString();
                    auto data = Data();
                    data.ParseFromString(val);
                    it->Next();
                    auto nextKeyString = it->key().ToString();
                    std::cout << "writing value " << keyString << std::endl;
                    writer->Write(data);

                }

                delete it;

                std::cout << "return OK status " << std::endl;

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


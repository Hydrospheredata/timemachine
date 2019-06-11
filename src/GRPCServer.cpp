//
// Created by Dmitry Isaev on 2019-01-30.
//

#include "GRPCServer.h"
#include <functional>
#include "spdlog/spdlog.h"
#include <shared_mutex>
#include "reqstore_service.grpc.pb.h"

using namespace rocksdb;

namespace hydrosphere
{
namespace reqstore
{

GRPCServer::GRPCServer()
{
    ropt = ReadOptions();
}

grpc::Status
GRPCServer::PerformIfExists(std::string baseName, std::function<grpc::Status(rocksdb::ColumnFamilyHandle *)> fn){
     try
    {
        auto cf = client->GetColumnFamily(baseName);
        if(!cf){
            spdlog::warn("Column family with name {} doesn't exists", baseName);
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Column family with name "+ baseName +" doesn't exists");
        }
        auto result = fn(cf);
        return result;
    }
    catch (const std::exception &e)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong: " + std::string(e.what()));
    }
    catch (...)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong");
    }
}

grpc::Status
GRPCServer::Perform(std::string baseName, std::function<grpc::Status(rocksdb::ColumnFamilyHandle *)> fn)
{

    try
    {
        auto cf = client->GetOrCreateColumnFamily(baseName);
        auto result = fn(cf);
        return result;
    }
    catch (const std::exception &e)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong: " + std::string(e.what()));
    }
    catch (...)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ouch!, something wrong");
    }
}

void GRPCServer::Init(std::shared_ptr<hydrosphere::reqstore::DbClient> _db)
{
    client = _db;
}

grpc::Status GRPCServer::Save(ServerContext *context, const SaveRequest *request, ID *response)
{

    std::function<grpc::Status(ColumnFamilyHandle * cf)> action = [&](ColumnFamilyHandle *cf) -> grpc::Status {
        auto id = client->GenerateId(request->folder(), request->timestamp());
        hydrosphere::reqstore::Data data;

        hydrosphere::reqstore::ID *id_ptr = &id;
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

        if (!putStatus.ok())
        {
            spdlog::error("PUT status is {0}", putStatus.ToString());

            return grpc::Status(grpc::StatusCode::INTERNAL, "Something wrong :( " + putStatus.ToString());
        }
        spdlog::debug("Saved data  with key ({0}, {1}) to folder {2}",
                      data.id().timestamp(),
                      data.id().unique(),
                      request->folder());

        response->set_timestamp(id.timestamp());
        response->set_unique(id.unique());
        return grpc::Status::OK;
    };

    grpc::Status status = Perform(request->folder(), action);

    return status;
}

grpc::Status GRPCServer::Get(ServerContext *context, const hydrosphere::reqstore::GetRequest *request, Data *response)
{
    auto status = PerformIfExists(request->folder(), [&](ColumnFamilyHandle *cf) -> grpc::Status {

        spdlog::debug("Get data by key ({0}, {1}) from folder {2}",
                      request->timestamp(),
                      request->unique(),
                      request->folder());

        std::string data;
        char bytes[16];
        hydrosphere::reqstore::ID req_id;
        req_id.set_timestamp(request->timestamp());
        req_id.set_unique(request->unique());
        RepositoryUtils::SerializeID(&req_id, bytes);
        auto bynaryId = rocksdb::Slice(bytes, 16);
        rocksdb::Status getStatus = client->Get(ropt, cf, bynaryId, &data);
        spdlog::debug("GET status is {0}", getStatus.ToString());
        if (!getStatus.ok())
        {
            spdlog::error("GET status is  {0}", getStatus.ToString());
            return grpc::Status(grpc::StatusCode::INTERNAL, "Something wrong :( " + getStatus.ToString());
        }
        hydrosphere::reqstore::Data result;
        result.set_data(data);
        result.mutable_id()->set_timestamp(request->timestamp());
        result.mutable_id()->set_unique(request->unique());
        response->ParseFromString(data);
        return grpc::Status::OK;
    });

    return status;
}

grpc::Status GRPCServer::GetRange(ServerContext *context, const RangeRequest *request, DataWriter *writer)
{
    auto status = PerformIfExists(request->folder(), [&](ColumnFamilyHandle *cf) -> grpc::Status {
        spdlog::debug("Get range from {0} to {1}", request->from(), request->till());

        std::string data;

        spdlog::debug("getting iterator from {0} to {1}", request->from(), request->till());

        client->Iter(ropt, cf, request, [&](hydrosphere::reqstore::ID key, hydrosphere::reqstore::Data data, bool stop) -> unsigned long int {
            if (context->IsCancelled())
                stop = true;
            writer->Write(data);
            return data.SerializeAsString().size();
        });

        return grpc::Status::OK;
    });

    return status;
}

grpc::Status GRPCServer::GetSubsample(ServerContext *ctx, const SubsampleRequest *request, DataWriter *dw)
{

    auto status = PerformIfExists(request->folder(), [&](ColumnFamilyHandle *cf) -> grpc::Status {

        spdlog::debug("Get subsample");

        int till = 0;
        int from = 0;

        if (request->type().has_period_request())
        {
            auto range = client->GetUniqueRangeForTS(cf, request->type().period_request().from(), request->type().period_request().till());
            from = range.from;
            till = range.till;
        }
        else
        {
            auto range = client->GetUniqueRange(cf);
            till = range.till;
            from = range.from;
        }

        int step = request->batch_size();
        if (step == 0)
            step = 100;

        auto amount = request->amount();
        if (step > amount)
        {
            step = amount;
        }

        spdlog::debug("step: {}, amount: {}", step, amount);

        for (int i = amount; i > 0; i = i - step)
        {
            int currStep = step;
            if (currStep < i)
            {
                currStep = i;
            }

            std::vector<rocksdb::Slice> keys;
            std::vector<std::string> result;
            std::vector<rocksdb::Status> statuses;
            char key[16 * step];

            for (int j = 0; j < step; j++)
            {
                auto r = rand();
                unsigned long random = ((till - from) * (r / (double)RAND_MAX)) + from;
                hydrosphere::reqstore::ID id;
                id.set_unique(random);
                id.set_timestamp(0);
                spdlog::debug("random:{}, from:{}, till: {}", random, from, till);
                spdlog::debug("ID(ts:{}, unique:{})", id.timestamp(), id.unique());

                RepositoryUtils::SerializeID(&id, &key[j*16]);
                auto slice = rocksdb::Slice(&key[j*16], 16);
                keys.push_back(slice);

            }

            statuses = client->GetBatch(rocksdb::ReadOptions(), cf, keys, &result);

            for (int s = 0; s < statuses.size(); s++)
            {
                auto status = statuses[s];
                if (!status.ok())
                {
                    spdlog::error("GET status is  {0}", status.ToString());
                }
                auto bytes = result[s];

                auto data = Data();
                data.ParseFromString(bytes);

                dw->Write(data);
            }
        }

        return grpc::Status::OK;
    });

    return status;
}

} // namespace reqstore
} // namespace hydrosphere

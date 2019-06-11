#include "SubsamplingHandler.h"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../DbClient.h"
#include "Poco/BinaryWriter.h"

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{

SubsamplingHandler::SubsamplingHandler(
    std::shared_ptr<hydrosphere::reqstore::DbClient> _client,
    std::string &&_name,
    unsigned long int _from,
    unsigned long int _till,
    unsigned int _amount,
    unsigned int _batchSize) : client(_client), name(_name), from(_from), till(_till), amount(_amount), batchSize(_batchSize)
{
    readOptions = rocksdb::ReadOptions();
}

void SubsamplingHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                       Poco::Net::HTTPServerResponse &response)
{
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/octet-stream");
    response.setChunkedTransferEncoding(true);

    response.set("Access-Control-Allow-Headers", "application/octet-stream");
    response.set("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE");
    response.set("Access-Control-Allow-Origin", "*");
    response.set("Allow", "POST, GET, OPTIONS, PUT, DELETE");

    auto cf = client->GetColumnFamily(name);
    std::ostream &ostr = response.send();
    auto br = Poco::BinaryWriter(ostr, Poco::BinaryWriter::NETWORK_BYTE_ORDER);
    
    if (!cf)
    {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
    }
    else
    {
       
        hydrosphere::reqstore::SubsampleRequestType type;

        unsigned long from_inc = 0;
        unsigned long till_inc = 0;

        if (from == 0 && till == 0)
        { // search by all range
            spdlog::debug("TotalSubsampleRequest subsampling");
            hydrosphere::reqstore::TotalSubsampleRequest total;
            auto range = client->GetUniqueRange(cf);
            from_inc = range.from;
            till_inc = range.till;
        }
        else
        {
            spdlog::debug("PeriodSubsampleRequest subsampling");
            hydrosphere::reqstore::PeriodSubsampleRequest period;
            auto range = client->GetUniqueRangeForTS(cf, from, till);
            from_inc = range.from;
            till_inc = range.till;
            period.set_from(from_inc);
            period.set_till(till_inc);
        }

        spdlog::debug("from: {}, till: {}", from_inc, till_inc);

        int step = batchSize;
        if (step == 0)
            step = 100;

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
                unsigned long random = ((till_inc - from_inc) * (r / (double)RAND_MAX)) + from_inc;
                hydrosphere::reqstore::ID id;
                id.set_unique(random);
                id.set_timestamp(0);

                spdlog::debug("ID(ts:{}, unique:{})", id.timestamp(), id.unique());
                spdlog::debug("random:{}, from:{}, till: {}", random, from_inc, till_inc);

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

                auto key = data.id();
                auto body = data.data();
                int size_ = body.size();

                long int ts_ = (long)key.timestamp();
                long int unique_ = (long)key.unique();
          
                br << ts_ << unique_ << size_;
                ostr << body;
            }
        }

        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
    }
}

} // namespace handlers
} // namespace reqstore
} // namespace hydrosphere
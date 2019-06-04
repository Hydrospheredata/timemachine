//
// Created by Dmitry Isaev on 2019-03-12.
//

#include "GetRangeHandler.h"
#include "../IDComparator.h"
#include "spdlog/spdlog.h"
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../utils/RepositoryUtils.h"
#include "Poco/BinaryWriter.h"
#include "Poco/Net/MessageHeader.h"

using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::MessageHeader;

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{

GetRangeHandler::GetRangeHandler(
    std::shared_ptr<hydrosphere::reqstore::DbClient> _client,
    std::string &&_name,
    unsigned long int _from,
    unsigned long int _till,
    unsigned long int _max_messages,
    unsigned long int _max_bytes,
    bool _reverse) : client(_client), name(_name), from(_from), till(_till), maxBytes(_max_bytes), maxMessages(_max_messages), reverse(_reverse), hydrosphere::reqstore::utils::RepositoryUtils()
{
    readOptions = rocksdb::ReadOptions();
}

void GetRangeHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/octet-stream");
    response.setChunkedTransferEncoding(true);

    response.set("Access-Control-Allow-Headers", "application/octet-stream");
    response.set("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE");
    response.set("Access-Control-Allow-Origin", "*");
    response.set("Allow", "POST, GET, OPTIONS, PUT, DELETE");

    std::ostream &ostr = response.send();

    spdlog::debug("Get range from {0} to {1}", from, till);
    auto cf = client->GetOrCreateColumnFamily(name);

    spdlog::debug("getting iterator from {0} to {1}", from, till);

    ID keyFrom;
    keyFrom.set_timestamp(from);

    char bytes[16];
    SerializeID(&keyFrom, bytes);
    auto keyFromSlice = rocksdb::Slice(bytes, 16);

    spdlog::debug("iterating with: reverse = {}, from = {}, till = {}, maxMessages = {}, maxBytes = {}", reverse, from, till, maxMessages, maxBytes);

    hydrosphere::reqstore::RangeRequest req;
    req.set_from(from);
    req.set_till(till);
    req.set_reverse(reverse);
    req.set_maxmessages(maxMessages);
    req.set_maxbytes(maxBytes);

    const hydrosphere::reqstore::RangeRequest *reqRef = &req;

    client->Iter(readOptions, cf, reqRef, [&](hydrosphere::reqstore::ID key, hydrosphere::reqstore::Data data, bool stopIt) -> unsigned long int {
        auto ulongSize = sizeof(long int);
        auto uintSize = sizeof(int);
        auto totalSize = ulongSize * 2 + uintSize;
        long int ts_ = (long)key.timestamp();
        long int unique_ = (long)key.unique();
        auto body = data.data();
        int size_ = body.size();

        spdlog::debug("batch to send: \n ts: {} \n unique: {} \n size: {}", ts_, unique_, size_);
        auto br = Poco::BinaryWriter(ostr, Poco::BinaryWriter::NETWORK_BYTE_ORDER);
        br << ts_ << unique_ << size_;
        ostr << body;

        return size_ + ulongSize + ulongSize + uintSize;
    });

    response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
}

} // namespace handlers
} // namespace reqstore
} // namespace hydrosphere

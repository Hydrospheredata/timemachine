//
// Created by Dmitry Isaev on 2019-04-17.
//

#include "StatusHandler.h"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../DbClient.h"
#include <Poco/JSON/Object.h>
#include "rocksdb/db.h"

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{

StatusHandler::StatusHandler(std::shared_ptr<hydrosphere::reqstore::DbClient> _client) : client(_client) {}

void StatusHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                  Poco::Net::HTTPServerResponse &response)
{
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    response.set("Access-Control-Allow-Headers", "application/json");
    response.set("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE");
    response.set("Access-Control-Allow-Origin", "*");
    response.set("Allow", "POST, GET, OPTIONS, PUT, DELETE");

    std::ostream &ostr = response.send();

    Poco::JSON::Array json;
    std::ostringstream oss;

    for (rocksdb::ColumnFamilyHandle *handle : client->handles)
    {
        json.add(handle->GetName());
    }

    json.stringify(oss);
    auto r = oss.str();
    ostr << r;
    response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
}

}; // namespace handlers

} // namespace reqstore
} // namespace hydrosphere

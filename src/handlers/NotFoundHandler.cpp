//
// Created by Dmitry Isaev on 2019-03-12.
//

#include "NotFoundHandler.h"
#include <Poco/Net/HTTPServerResponse.h>
#include "spdlog/spdlog.h"

using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{

void NotFoundHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    std::ostream &out = response.send();
    out << "Page Not Found";
    spdlog::warn("Page Not Found"); // TODO: Log path
    response.setStatus(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
}

} // namespace handlers
} // namespace reqstore
} // namespace hydrosphere

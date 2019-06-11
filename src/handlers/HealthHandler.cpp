//
// Created by Dmitry Isaev on 2019-03-06.
//

#include "HealthHandler.h"
#include <Poco/Net/HTTPServerResponse.h>

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{
void HealthHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    //TODO: check DB status
    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");
    std::ostream &ostr = response.send();
    ostr << "OK";
    response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
}
} // namespace handlers
} // namespace reqstore
} // namespace hydrosphere
//
// Created by Dmitry Isaev on 2019-03-06.
//

#include <Poco/Net/HTTPRequestHandler.h>

#ifndef REQSTORE_HEALTHHANDLER_H
#define REQSTORE_HEALTHHANDLER_H

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{
class HealthHandler : public Poco::Net::HTTPRequestHandler
{

private:
    void handleRequest(Poco::Net::HTTPServerRequest &request,
                       Poco::Net::HTTPServerResponse &response) override;
};
} // namespace handlers
} // namespace reqstore
} // namespace hydrosphere

#endif //REQSTORE_HEALTHHANDLER_H

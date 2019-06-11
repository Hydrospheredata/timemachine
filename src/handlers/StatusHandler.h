//
// Created by Dmitry Isaev on 2019-04-17.
//

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../DbClient.h"

#ifndef REQSTORE_STATUSHANDLER_H
#define REQSTORE_STATUSHANDLER_H

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{

class StatusHandler : public Poco::Net::HTTPRequestHandler
{

public:
    StatusHandler(std::shared_ptr<hydrosphere::reqstore::DbClient>);

private:
    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

    std::shared_ptr<hydrosphere::reqstore::DbClient> client;
};

}; // namespace handlers

} // namespace reqstore
} // namespace hydrosphere

#endif //REQSTORE_STATUSHANDLER_H

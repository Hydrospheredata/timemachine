#pragma once

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <memory>
#include "../DbClient.h"

#ifndef REQSTORE_HEALTH_HANDLER_H
#define REQSTORE_HEALTH_HANDLER_H

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{
class HandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{

public:
    HandlerFactory(std::shared_ptr<hydrosphere::reqstore::DbClient>);

private:
    Poco::Net::HTTPRequestHandler *createRequestHandler(
        const Poco::Net::HTTPServerRequest &request) override;

    std::shared_ptr<hydrosphere::reqstore::DbClient> client;
};
} // namespace handlers
} // namespace reqstore
} // namespace hydrosphere

#endif
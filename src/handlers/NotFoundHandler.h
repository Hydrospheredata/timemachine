//
// Created by Dmitry Isaev on 2019-03-12.
//

#include <Poco/Net/HTTPRequestHandler.h>

#ifndef REQSTORE_NOTFOUNDHANDLER_H
#define REQSTORE_NOTFOUNDHANDLER_H

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{

class NotFoundHandler : public Poco::Net::HTTPRequestHandler
{

private:
    void handleRequest(Poco::Net::HTTPServerRequest &request,
                       Poco::Net::HTTPServerResponse &response) override;
};

} // namespace handlers
} // namespace reqstore
} // namespace hydrosphere

#endif //REQSTORE_NOTFOUNDHANDLER_H

//
// Created by Dmitry Isaev on 2019-03-06.
//

#include <Poco/Net/HTTPRequestHandler.h>

#ifndef REQSTORE_INFOHANDLER_H
#define REQSTORE_INFOHANDLER_H

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{
class InfoHandler : public Poco::Net::HTTPRequestHandler
{

private:
    void handleRequest(Poco::Net::HTTPServerRequest &request,
                       Poco::Net::HTTPServerResponse &response) override;
};
} // namespace handlers
} // namespace reqstore
} // namespace hydrosphere

#endif //REQSTORE_INFOHANDLER_H

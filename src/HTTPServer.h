#include "DbClient.h"
#include <Poco/Util/ServerApplication.h>

#ifndef REQSTORE_HTTP_SERVER_H
#define REQSTORE_HTTP_SERVER_H

namespace hydrosphere
{
namespace reqstore
{

class HTTPServer : public Poco::Util::ServerApplication
{

public:
    void start(std::shared_ptr<hydrosphere::reqstore::DbClient>);

private:
    std::shared_ptr<hydrosphere::reqstore::DbClient> client;
};

} // namespace reqstore
} // namespace hydrosphere

#endif
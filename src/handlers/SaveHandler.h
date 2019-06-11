//
// Created by Dmitry Isaev on 2019-03-06.
//

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../utils/RepositoryUtils.h"
#include "../DbClient.h"
#include "rocksdb/options.h"

#ifndef REQSTORE_SAVEHANDLER_H
#define REQSTORE_SAVEHANDLER_H

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{
class SaveHandler : public Poco::Net::HTTPRequestHandler, hydrosphere::reqstore::utils::RepositoryUtils
{

public:
    SaveHandler(std::shared_ptr<hydrosphere::reqstore::DbClient>, std::string &&, bool, unsigned long);

private:
    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
    unsigned long timestamp;
    std::shared_ptr<hydrosphere::reqstore::DbClient> client;
    std::string name;
    rocksdb::WriteOptions wopt;
};
} // namespace handlers
} // namespace reqstore
} // namespace hydrosphere

#endif //REQSTORE_SAVEHANDLER_H

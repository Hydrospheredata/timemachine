#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../utils/RepositoryUtils.h"
#include "../DbClient.h"
#include "rocksdb/options.h"

#ifndef REQSTORE_SUBSAMPLINGHANDLER_H
#define REQSTORE_SUBSAMPLINGHANDLER_H

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{

class SubsamplingHandler : public Poco::Net::HTTPRequestHandler, hydrosphere::reqstore::utils::RepositoryUtils
{

public:
    SubsamplingHandler(std::shared_ptr<hydrosphere::reqstore::DbClient>,
                       std::string &&,
                       unsigned long int,
                       unsigned long int,
                       unsigned int,
                       unsigned int);

private:
    std::shared_ptr<hydrosphere::reqstore::DbClient> client;
    std::string name;
    unsigned long int from;
    unsigned long int till;
    unsigned int amount;
    unsigned int batchSize;
    rocksdb::ReadOptions readOptions;
    void handleRequest(Poco::Net::HTTPServerRequest &request,
                       Poco::Net::HTTPServerResponse &response) override;
};
} // namespace handlers
} // namespace reqstore
} // namespace hydrosphere

#endif //REQSTORE_SUBSAMPLINGHANDLER_H

//
// Created by Dmitry Isaev on 2019-03-12.
//

#include <Poco/Net/HTTPRequestHandler.h>
#include "../utils/RepositoryUtils.h"
#include "../DbClient.h"
#include "rocksdb/options.h"

#ifndef REQSTORE_GETRANGEHANDLER_H
#define REQSTORE_GETRANGEHANDLER_H

namespace hydrosphere
{
namespace reqstore
{
namespace handlers
{

class GetRangeHandler : public Poco::Net::HTTPRequestHandler, hydrosphere::reqstore::utils::RepositoryUtils
{

public:
    GetRangeHandler(std::shared_ptr<hydrosphere::reqstore::DbClient>,
                    std::string &&,
                    unsigned long int,
                    unsigned long int,
                    unsigned long int,
                    unsigned long int,
                    bool);

private:
    std::shared_ptr<hydrosphere::reqstore::DbClient> client;
    std::string name;
    unsigned long int from;
    unsigned long int till;
    unsigned long int maxMessages;
    unsigned long int maxBytes;
    bool reverse;
    rocksdb::ReadOptions readOptions;
    void handleRequest(Poco::Net::HTTPServerRequest &request,
                       Poco::Net::HTTPServerResponse &response) override;
};
} // namespace handlers
} // namespace reqstore
} // namespace hydrosphere

#endif //REQSTORE_GETRANGEHANDLER_H

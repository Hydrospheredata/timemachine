//
// Created by Dmitry Isaev on 2019-03-12.
//

#include <Poco/Net/HTTPRequestHandler.h>
#include "../utils/RepositoryUtils.h"
#include "../DbClient.h"
#include "rocksdb/options.h"

#ifndef TIMEMACHINE_GETRANGEHANDLER_H
#define TIMEMACHINE_GETRANGEHANDLER_H

namespace timemachine {
    namespace handlers {

        class GetRangeHandler: public Poco::Net::HTTPRequestHandler, timemachine::utils::RepositoryUtils {

        public:
            GetRangeHandler(std::shared_ptr<timemachine::DbClient>,
                            std::string&&,
                            unsigned long int,
                            unsigned long int);

        private:
            std::shared_ptr <timemachine::DbClient> client;
            std::string name;
            unsigned long int from;
            unsigned long int till;
            rocksdb::ReadOptions readOptions;
            void handleRequest(Poco::Net::HTTPServerRequest &request,
                               Poco::Net::HTTPServerResponse &response) override;

        };
    }
}


#endif //TIMEMACHINE_GETRANGEHANDLER_H

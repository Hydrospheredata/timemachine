//
// Created by Dmitry Isaev on 2019-03-06.
//

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../utils/RepositoryUtils.h"
#include "../DbClient.h"
#include "rocksdb/options.h"

#ifndef TIMEMACHINE_SAVEHANDLER_H
#define TIMEMACHINE_SAVEHANDLER_H


namespace timemachine {
    namespace handlers {
    class SaveHandler : public Poco::Net::HTTPRequestHandler, timemachine::utils::RepositoryUtils {

        public:
            SaveHandler(std::shared_ptr<timemachine::DbClient>, std::string&&, bool);

        private:
            void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

            std::shared_ptr <timemachine::DbClient> client;
            std::string name;
            rocksdb::WriteOptions wopt;

        };
    }
}


#endif //TIMEMACHINE_SAVEHANDLER_H

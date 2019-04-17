//
// Created by Dmitry Isaev on 2019-04-17.
//


#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../DbClient.h"

#ifndef TIMEMACHINE_STATUSHANDLER_H
#define TIMEMACHINE_STATUSHANDLER_H

namespace timemachine {
    namespace handlers {

        class StatusHandler : public Poco::Net::HTTPRequestHandler {

        public:
            StatusHandler(std::shared_ptr<timemachine::DbClient>);

        private:
            void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

            std::shared_ptr<timemachine::DbClient> client;

        };

    };

}



#endif //TIMEMACHINE_STATUSHANDLER_H

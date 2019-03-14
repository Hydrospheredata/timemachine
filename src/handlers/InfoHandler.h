//
// Created by Dmitry Isaev on 2019-03-06.
//

#include <Poco/Net/HTTPRequestHandler.h>

#ifndef TIMEMACHINE_INFOHANDLER_H
#define TIMEMACHINE_INFOHANDLER_H


namespace timemachine {
    namespace handlers {
        class InfoHandler : public Poco::Net::HTTPRequestHandler {

        private:
            void handleRequest(Poco::Net::HTTPServerRequest &request,
                               Poco::Net::HTTPServerResponse &response) override;

        };
    }
}


#endif //TIMEMACHINE_INFOHANDLER_H

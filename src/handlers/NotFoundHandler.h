//
// Created by Dmitry Isaev on 2019-03-12.
//

#include <Poco/Net/HTTPRequestHandler.h>

#ifndef TIMEMACHINE_NOTFOUNDHANDLER_H
#define TIMEMACHINE_NOTFOUNDHANDLER_H

namespace timemachine {
    namespace handlers {

        class NotFoundHandler : public Poco::Net::HTTPRequestHandler {

        private:
            void handleRequest(Poco::Net::HTTPServerRequest &request,
                               Poco::Net::HTTPServerResponse &response) override;

        };

    }
}


#endif //TIMEMACHINE_NOTFOUNDHANDLER_H

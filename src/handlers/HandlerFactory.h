#pragma once

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <memory>
#include "../DbClient.h"

#ifndef TIMEMACHINE_HEALTH_HANDLER_H
#define TIMEMACHINE_HEALTH_HANDLER_H

namespace timemachine {
    namespace handlers {
        class HandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {

        public:
            HandlerFactory(std::shared_ptr<timemachine::DbClient>);

        private:
            Poco::Net::HTTPRequestHandler *createRequestHandler(
                    const Poco::Net::HTTPServerRequest &request) override;

            std::shared_ptr<timemachine::DbClient> client;
        };
    }
}

#endif
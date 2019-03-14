#include "DbClient.h"
#include <Poco/Util/ServerApplication.h>

#ifndef TIMEMACHINE_HTTP_SERVER_H
#define TIMEMACHINE_HTTP_SERVER_H


namespace timemachine {

    class HTTPServer : public Poco::Util::ServerApplication {

    public:
        void start(std::shared_ptr<timemachine::DbClient>);

    private:
        std::shared_ptr<timemachine::DbClient> client;

    };

}

#endif
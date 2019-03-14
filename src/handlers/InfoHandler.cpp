//
// Created by Dmitry Isaev on 2019-03-06.
//

#include "InfoHandler.h"
#include <Poco/Net/HTTPServerResponse.h>

using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;


namespace timemachine {
    namespace handlers {

        void InfoHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {

            response.setChunkedTransferEncoding(true);
            response.setContentType("text/html");
            std::ostream &ostr = response.send();
            ostr << "timemachine 0.0.1"; //TODO: get version number from def in cmake
            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);

        };


    }
}
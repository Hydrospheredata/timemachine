#include "HandlerFactory.h"
#include "HealthHandler.h"
#include "InfoHandler.h"
#include "SaveHandler.h"
#include <Poco/Net/HTTPServerRequest.h>
#include "Poco/URI.h"
#include "Poco/Path.h"
#include "spdlog/spdlog.h"
#include "NotFoundHandler.h"
#include "GetRangeHandler.h"


namespace timemachine {
    namespace handlers {

        HandlerFactory::HandlerFactory(std::shared_ptr<timemachine::DbClient> _client) : client(_client) {}

        Poco::Net::HTTPRequestHandler *
        HandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &request) {

            Poco::URI uri(request.getURI());
            Poco::Path path(uri.getPath());

            spdlog::debug("request to endpoint {}", request.getURI());

            if (request.getURI() == "/" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
                spdlog::debug("handler:  InfoHandler");
                return new InfoHandler;
            } else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST && path.depth() == 1 && path[1] == "put" ) {
                spdlog::debug("handler:  SaveHandler, depth: {}, path: {} -> {}", path.depth(), path[0], path[1]);
                auto name = path[0];
                return new SaveHandler(client, std::move(name));
            }  else if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET && path.depth() == 1 && path[1] == "get" ){
                spdlog::debug("handler:  GetRangeHandler, depth: {}, path: {} -> {}", path.depth(), path[0], path[1]);
                auto name = path[0];
                auto query = uri.getQueryParameters();
                auto from = 0;
                //TODO: unsigned long???
                auto till = std::numeric_limits<int>::max();

                for (std::pair < std::string, std::string > &kv: query) {
                    if(kv.first == "from"){
                        //TODO: stoul!
                        from = std::stoi(kv.second);
                    }
                    if(kv.second == "to"){
                        //TODO: stoul!
                        till = std::stoi(kv.second);
                    }
                }

                return new GetRangeHandler(client, std::move(name), from, till);

            } else if (request.getURI() == "/health" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
                return new HealthHandler;
            }
            else {
                return new NotFoundHandler;
            }

            return nullptr;
        }

    }

}
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
                auto query = uri.getQueryParameters();

                bool useWAL = false;

                for (std::pair < std::string, std::string > &kv: query) {
                    if(kv.first == "useWAL" && (kv.second == "true" || kv.second == "1")){
                        useWAL = true;
                    }
                }

                return new SaveHandler(client, std::move(name), useWAL);
            }  else if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET && path.depth() == 1 && path[1] == "get" ){
                spdlog::debug("handler:  GetRangeHandler, depth: {}, path: {} -> {}", path.depth(), path[0], path[1]);
                auto name = path[0];
                auto query = uri.getQueryParameters();
                unsigned long int from = 0;
                unsigned long int till = std::numeric_limits<unsigned long int>::max();
                for (std::pair < std::string, std::string > &kv: query) {
                    if(kv.first == "from"){
                        spdlog::debug("kv: {} -> {}", kv.first, kv.second);
                        auto from_ = std::stoul(kv.second);
                        if(from_ != -1) from = from_;
                    }
                    if(kv.first == "to"){
                        spdlog::debug("kv: {} -> {}", kv.first, kv.second);
                        auto till_ = std::stoul(kv.second);
                        if(till_ != -1) till = till_;
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
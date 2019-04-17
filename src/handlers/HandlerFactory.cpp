#include "HandlerFactory.h"
#include "HealthHandler.h"
#include "InfoHandler.h"
#include "SaveHandler.h"
#include "StatusHandler.h"
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

            if ((request.getURI() == "/" || request.getURI() == "" ) && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
                return new InfoHandler;
            } else if(request.getURI() == "/status" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
                spdlog::debug("handler:  StatusHandler");
                return new StatusHandler(client);
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
                spdlog::debug("uri.getQueryParameters");
                for (std::pair < std::string, std::string > &kv: query){
                    spdlog::debug("{} ->{}", kv.first, kv.second);
                }
                unsigned long int from = 0;
                unsigned long int till = std::numeric_limits<unsigned long int>::max();
                unsigned long int maxMessages = 0;
                unsigned long int maxBytes = 0;
                bool reverse = false;
                
                for (std::pair < std::string, std::string > &kv: query) {
                    std::string from_s = "from";
                    std::string to_s = "to";
                    std::string maxBytes_s = "maxBytes";
                    std::string maxMessages_s = "maxMessages";
                    std::string reverse_s = "reverse";
                    if(from_s.compare(kv.first) == 0){
                        spdlog::debug("kv: {} -> {}", kv.first, kv.second);
                        auto from_ = std::stoul(kv.second);
                        if(from_ != -1) from = from_;
                    }
                    if(to_s.compare(kv.first) == 0){
                        spdlog::debug("kv: {} -> {}", kv.first, kv.second);
                        auto till_ = std::stoul(kv.second);
                        if(till_ != -1) till = till_;
                    }
                    if(maxBytes_s.compare(kv.first) == 0){
                        spdlog::debug("kv: {} -> {}", kv.first, kv.second);
                        auto maxBytes_ = std::stoul(kv.second);
                        if(maxBytes_ != -1) maxBytes = maxBytes_;
                    }
                    if(maxMessages_s.compare(kv.first) == 0){
                        spdlog::debug("kv: {} -> {}", kv.first, kv.second);
                        auto maxMessages_ = std::stoul(kv.second);
                        if(maxMessages_ != -1) maxMessages = maxMessages_;
                    }
                    if(reverse_s.compare(kv.first) == 0){
                        spdlog::debug("kv: {} -> {}", kv.first, kv.second);
                        std::string t = "true";
                        if(kv.second.compare(t) == 0) reverse = true;
                    }
                }

                spdlog::debug("return new GetRangeHandler with params: reverse = {}, from = {}, till = {}, maxMessages = {}, maxBytes = {}", reverse, from, till, maxMessages, maxBytes);

                return new GetRangeHandler(client, std::move(name), from, till, maxMessages, maxBytes, reverse);

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
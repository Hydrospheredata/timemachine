//
// Created by Dmitry Isaev on 2019-03-06.
//

#include "SaveHandler.h"
#include "../DbClient.h"
#include "spdlog/spdlog.h"
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../utils/RepositoryUtils.h"
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/BinaryReader.h>


namespace timemachine {
    namespace handlers {

        SaveHandler::SaveHandler(std::shared_ptr<timemachine::DbClient>_client, std::string&& _name, bool useWAL):
                client(_client), name(_name), timemachine::utils::RepositoryUtils() {
            wopt = rocksdb::WriteOptions();
            wopt.disableWAL = !useWAL;
        }

        void SaveHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
            response.setChunkedTransferEncoding(true);
            response.setContentType("application/json");
            std::ostream &ostr = response.send();

            try {
                auto id = client->GenerateId();

                timemachine::Data data;

                std::istream& stream = request.stream();
                std::string string(std::istreambuf_iterator<char>(stream), {});
                data.set_data(string);
                data.mutable_id()->set_timestamp(id.timestamp());
                data.mutable_id()->set_unique(id.unique());

                auto cf = client->GetOrCreateColumnFamily(name);
                char bytes[16];
                SerializeID(&id, bytes);
                auto bynaryId = rocksdb::Slice(bytes, 16);
                spdlog::debug("id serialized");
                spdlog::debug("saving data with size: {}", string.size());
                spdlog::debug("saving data with size revert: {}", data.data().size());

                rocksdb::Status putStatus = client->Put(wopt, cf, bynaryId, data.SerializeAsString());
                Poco::JSON::Object json;
                std::ostringstream oss;
                Poco::UInt64 ts_p = id.timestamp();
                json.set("ts", ts_p);
                Poco::UInt64 u_p = id.unique();
                json.set("uniq", u_p);
                json.stringify(oss);
                 auto r = oss.str();

                spdlog::debug("PUT status is {0}, id({1}, {2}) to folder",
                              putStatus.ToString(),
                              id.timestamp(),
                              id.unique(),
                              name);


                ostr << r;                    
                response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);

                if (!putStatus.ok()) {
                    spdlog::error("PUT status is {0}", putStatus.ToString());
                    response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
                    spdlog::debug("Saved data  with key ({0}, {1}) to folder {2}",
                                  id.timestamp(),
                                  id.unique(),
                                  name
                    );
                }
            } catch (const std::exception &e) {
                std::ostream &ostr = response.send();
                ostr << e.what();
                response.setStatus(Poco::Net::HTTPServerResponse::HTTP_INTERNAL_SERVER_ERROR);
            } catch (...) {
                response.setStatus(Poco::Net::HTTPServerResponse::HTTP_INTERNAL_SERVER_ERROR);
            }

        };


    }
}
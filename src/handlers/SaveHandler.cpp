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


namespace timemachine {
    namespace handlers {

        SaveHandler::SaveHandler(std::shared_ptr<timemachine::DbClient>_client, std::string&& _name):
                client(_client), name(_name), timemachine::utils::RepositoryUtils() {
            wopt = rocksdb::WriteOptions();
            wopt.disableWAL = true;
        }

        void SaveHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
            response.setChunkedTransferEncoding(true);
            response.setContentType("application/json");
            std::ostream &ostr = response.send();

            try {
                auto id = client->GenerateId(name);

                timemachine::Data data;

                std::istream& stream = request.stream();
                std::string string(std::istreambuf_iterator<char>(stream), {});
                data.set_data(string);
                data.mutable_id()->set_folder(id.folder());
                data.mutable_id()->set_timestamp(id.timestamp());
                data.mutable_id()->set_unique(id.unique());

                auto cf = client->GetOrCreateColumnFamily(name);
                char bytes[16];
                SerializeID(&id, bytes);
                auto bynaryId = rocksdb::Slice(bytes, 16);
                spdlog::debug("id serialized");
                rocksdb::Status putStatus = client->db->Put(wopt, cf, bynaryId, data.SerializeAsString());
                spdlog::debug("PUT status is {0}, id({1}, {2}, {3})",
                              putStatus.ToString(),
                              id.folder(),
                              id.timestamp(),
                              id.unique());

                if (!putStatus.ok()) {
                    spdlog::error("PUT status is {0}", putStatus.ToString());
                    std::ostream& ostr = response.send();
                    Poco::JSON::Object::Ptr obj;
                    obj->set("ts", id.timestamp());
                    obj->set("uniq", id.unique());
                    Poco::JSON::Stringifier::stringify(obj, ostr);
                    response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
                    spdlog::debug("Saved data  with key ({0}, {1}, {2})",
                                  id.timestamp(),
                                  id.unique(),
                                  id.folder()
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
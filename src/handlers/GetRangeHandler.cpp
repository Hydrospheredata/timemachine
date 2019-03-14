//
// Created by Dmitry Isaev on 2019-03-12.
//

#include "GetRangeHandler.h"
#include "../IDComparator.h"
#include "spdlog/spdlog.h"
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../utils/RepositoryUtils.h"

using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;


namespace timemachine {
    namespace handlers {

        GetRangeHandler::GetRangeHandler(
                std::shared_ptr<timemachine::DbClient>_client,
                std::string&& _name,
                int _from,
                int _till) :
        client(_client), name(_name), from(_from), till(_till), timemachine::utils::RepositoryUtils(){
            readOptions = rocksdb::ReadOptions();
        }

        void GetRangeHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response){
            response.setChunkedTransferEncoding(true);
            response.setContentType("application/json");
            std::ostream &ostr = response.send();

            response.setChunkedTransferEncoding(true);

            spdlog::debug("Get range from {0} to {1}", from, till);
            auto cf = client->GetOrCreateColumnFamily(name);

            spdlog::debug("getting iterator from {0} to {1}", from, till);

            rocksdb::Iterator *it = client->db->NewIterator(readOptions, cf);

            ID keyFrom;
            keyFrom.set_folder(name);
            keyFrom.set_timestamp(from);
            auto keyFromString = keyFrom.SerializeAsString();

            if (from == 0) {
                spdlog::debug("seek from start");
                it->SeekToFirst();
            } else {
                spdlog::debug("seek from {}", from);
                it->SeekForPrev(keyFrom.SerializeAsString());
            }

            timemachine::IDComparator comparator;



            for (; it->Valid(); it->Next()) {

                auto keyString = it->key().ToString();
                auto key = ID();
                key.ParseFromString(keyString);

                spdlog::debug("till is {}", till);
                if (till != 0 && comparator.Compare(keyFromString, it->key()) < 0) break;

                spdlog::debug("iterating key ({0}, {1}, {2})",
                              key.timestamp(),
                              key.unique(),
                              key.folder()
                );

                auto val = it->value().ToString();
                ostr << val;

            }

            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);

            delete it;

            spdlog::debug("return {0} status", "OK");
        }

    }
}

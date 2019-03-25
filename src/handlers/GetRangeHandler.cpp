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
                unsigned long int _from,
                unsigned long int _till) :
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

            char bytes[16];
            SerializeID(&keyFrom, bytes);
            auto keyFromSlice = rocksdb::Slice(bytes, 16);

            if (from == 0) {
                spdlog::debug("seek from start");
                it->SeekToFirst();
            } else {
                spdlog::debug("seek from {}", keyFrom.timestamp());
                it->SeekForPrev(keyFromSlice);
            }

            for (; it->Valid(); it->Next()) {

                auto idSlice = it->key();
                auto key = DeserializeID(idSlice, name);

                spdlog::debug("till is {}", till);
                if (till != 0 && till < key.timestamp()) break;

                spdlog::debug("iterating key ({}, {}, {})",
                              key.timestamp(),
                              key.unique(),
                              key.folder()
                );

                auto val = it->value();

                char header[20];

                unsigned long int ts_ = key.timestamp();
                unsigned long int unique_ = key.unique();
                unsigned int size_ = it->value().size();

                std::memcpy(&ts_, header, 8);
                std::memcpy(&unique_, header + 8, 8);
                std::memcpy(&size_, header + 16, 4);

                ostr << header << val.data();
            }

            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);

            delete it;

            spdlog::debug("return {0} status", "OK");
        }

    }
}

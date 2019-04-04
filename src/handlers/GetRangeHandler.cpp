//
// Created by Dmitry Isaev on 2019-03-12.
//

#include "GetRangeHandler.h"
#include "../IDComparator.h"
#include "spdlog/spdlog.h"
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../utils/RepositoryUtils.h"
#include "Poco/BinaryWriter.h"

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
            response.setContentType("application/octet-stream");
            std::ostream &ostr = response.send();

            response.setChunkedTransferEncoding(true);

            spdlog::debug("Get range from {0} to {1}", from, till);
            auto cf = client->GetOrCreateColumnFamily(name);

            spdlog::debug("getting iterator from {0} to {1}", from, till);

            ID keyFrom;
            keyFrom.set_timestamp(from);

            char bytes[16];
            SerializeID(&keyFrom, bytes);
            auto keyFromSlice = rocksdb::Slice(bytes, 16);

            client->Iter(readOptions, cf, [&](rocksdb::Iterator* it)->void {

                if (from == 0) {
                    spdlog::debug("seek from start");
                    it->SeekToFirst();
                } else {
                    spdlog::debug("seek from {}", keyFrom.timestamp());
                    it->SeekForPrev(keyFromSlice);
                }

                for (; it->Valid(); it->Next()) {

                    auto idSlice = it->key();
                    auto key = DeserializeID(idSlice);

                    spdlog::debug("till is {}", till);
                    if (till != 0 && till < key.timestamp()) break;

                    spdlog::debug("iterating key ({}, {}) from folder {}",
                                key.timestamp(),
                                key.unique(),
                                name
                    );

                    auto val = it->value().data();

                    auto ulongSize = sizeof(long int);
                    auto uintSize = sizeof(int);
                    auto totalSize = ulongSize * 2 + uintSize;
                    long int ts_ = (long)key.timestamp();
                    long int unique_ = (long)key.unique();
                    timemachine::Data data;
                    auto dataString = it->value().ToString();
                    data.ParseFromString(dataString);
                    auto body = data.data();
                    int size_ = body.size();

                    spdlog::debug("batch to send: \n ts: {} \n unique: {} \n size: {}", ts_, unique_, size_);

                    auto br = Poco::BinaryWriter(ostr, Poco::BinaryWriter::NETWORK_BYTE_ORDER);
                    br << ts_ << unique_ << size_;
                    ostr << body;
                }
                response.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
                delete it;
                spdlog::debug("return {0} status", "OK");

            });

        }

    }
}

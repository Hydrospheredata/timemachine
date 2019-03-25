//
// Created by Dmitry Isaev on 2019-03-07.
//
#include <chrono>
#include <random>
#include <stdio.h>

#include "RepositoryUtils.h"
#include "timeMachine.grpc.pb.h"
#include "spdlog/spdlog.h"


namespace timemachine {

    namespace utils {

        void RepositoryUtils::SerializeID(const timemachine::ID* id, char* bytes) {
            unsigned long int ts = id->timestamp();
            unsigned long int unique = id->unique();
            std::memcpy(bytes, &ts, 8);
            std::memcpy(bytes + 8, &unique, 8);
        }

        timemachine::ID RepositoryUtils::DeserializeID(const rocksdb::Slice& slice, std::string& folder) {
            timemachine::ID id;
            id.set_folder(folder);
            auto bytes = slice.data();
            unsigned long int timestamp;
            unsigned long int unique;

            std::memcpy(&timestamp, bytes, 8);
            std::memcpy(&unique, bytes + 8, 8);

            id.set_unique(unique);
            id.set_timestamp(timestamp);

            return id;
        }

    }
}

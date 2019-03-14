//
// Created by Dmitry Isaev on 2019-03-07.
//

#include "RepositoryUtils.h"
#include "timeMachine.grpc.pb.h"
#include <chrono>
#include <random>


namespace timemachine {

    namespace utils {

        std::default_random_engine RepositoryUtils::generator;

        std::uniform_int_distribution<int> RepositoryUtils::distribution = std::uniform_int_distribution<int>(1, 2147483647);

        timemachine::ID RepositoryUtils::GenerateId(const std::string& folder) {
            auto ms = std::chrono::system_clock::now().time_since_epoch().count();

            timemachine::ID id;

            id.set_timestamp(ms);
            id.set_folder(folder);
            int unique = distribution(generator);
            id.set_unique(unique);

            return id;
        }

    }
}

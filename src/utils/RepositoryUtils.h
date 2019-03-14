//
// Created by Dmitry Isaev on 2019-03-07.
//

#include "timeMachine.grpc.pb.h"
#include <random>

#ifndef TIMEMACHINE_REPOSITORYUTILS_H
#define TIMEMACHINE_REPOSITORYUTILS_H

namespace timemachine {

    namespace utils {

        class RepositoryUtils {
        public:
            static timemachine::ID GenerateId(const std::string&);

        private:
            static std::default_random_engine generator;
            static std::uniform_int_distribution<int> distribution;
        };

    }
}



#endif //TIMEMACHINE_REPOSITORYUTILS_H

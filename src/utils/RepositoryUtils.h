//
// Created by Dmitry Isaev on 2019-03-07.
//

#include "timeMachine.grpc.pb.h"
#include "rocksdb/db.h"
#include <random>

#ifndef TIMEMACHINE_REPOSITORYUTILS_H
#define TIMEMACHINE_REPOSITORYUTILS_H

namespace timemachine {

    namespace utils {

        class RepositoryUtils {
        public:
            static void SerializeID(const timemachine::ID*, char*);
            static timemachine::ID DeserializeID(const rocksdb::Slice&, std::string&);

        };

    }
}



#endif //TIMEMACHINE_REPOSITORYUTILS_H

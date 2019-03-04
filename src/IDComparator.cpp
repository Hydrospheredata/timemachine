
#include "timeMachine.grpc.pb.h"
#include "rocksdb/db.h"
#include "IDComparator.h"
#include "spdlog/spdlog.h"

namespace timemachine {

    int IDComparator::Compare(const rocksdb::Slice &a, const rocksdb::Slice &b) const {
        int result = 0;
        auto k1 = ID();
        k1.ParseFromString(a.ToString());

        auto k2 = ID();
        k2.ParseFromString(b.ToString());

        spdlog::debug("comparing ({}, {}) and ({}, {})", k1.timestamp(),  k1.incremental(), k2.timestamp(), k2.incremental());

        if(k1.timestamp() < k2.timestamp()) result = -1;
        else if(k1.timestamp() > k2.timestamp()) result = 1;
        else if(k1.incremental() < k2.incremental()) result = -1;
        else if(k1.incremental() > k2.incremental()) result = 1;
        spdlog::debug("result is {}", result);
        return result;
    }

    const char *IDComparator::Name() const {
            return "IDComparator";
    }

    void IDComparator::FindShortestSeparator(std::string *start, const rocksdb::Slice &limit) const {

    }

    void IDComparator::FindShortSuccessor(std::string *key) const {

    }

    bool IDComparator::Equal(const rocksdb::Slice &a, const rocksdb::Slice &b) const {
        return Compare(a, b) == 0;
    }
}


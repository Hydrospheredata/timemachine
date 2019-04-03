
#include "timeMachine.grpc.pb.h"
#include "rocksdb/db.h"
#include "IDComparator.h"
#include "spdlog/spdlog.h"

namespace timemachine {

    int IDComparator::Compare(const rocksdb::Slice &a, const rocksdb::Slice &b) const {
        int result = 0;
        std::string name = "";
        auto k1 = DeserializeID(a);

        auto k2 = DeserializeID(b);

        if (k1.timestamp() < k2.timestamp()) result = -1;
        else if (k1.timestamp() > k2.timestamp()) result = 1;
        else if (k1.unique() < k2.unique()) result = -1;
        else if (k1.unique() > k2.unique()) result = 1;
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


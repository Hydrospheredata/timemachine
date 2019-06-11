
#include "reqstore_service.grpc.pb.h"
#include "rocksdb/db.h"
#include "IDComparator.h"
#include "spdlog/spdlog.h"

namespace hydrosphere
{
namespace reqstore
{

int IDComparator::Compare(const rocksdb::Slice &a, const rocksdb::Slice &b) const
{
    int result = 0;
    std::string name = "";
    auto k1 = DeserializeID(a);
    auto k2 = DeserializeID(b);

    result = compareByTs(k1, k2);

    // spdlog::debug("compareByTs! k1: {}, k2: {}, result is {}", k1.timestamp(), k2.timestamp(), result);

    if(result == 0){
        result = compareByUnique(k1, k2);
        // spdlog::debug("compareByUnique! k1: {}, k2: {}, result is {}", k1.unique(), k2.unique(), result);
    }

    
    return result;
}

int IDComparator::compareByTs(ID& first, ID& second) const{
    //if request ts is empty we should compare by unique (for subsampling)
    if(first.timestamp() == 0 || second.timestamp() == 0) return 0; 
    else if (first.timestamp() < second.timestamp()) return -1;
    else if (first.timestamp() > second.timestamp()) return 1;
    else return 0;
}

int IDComparator::compareByUnique(ID& first, ID& second) const{
    if (first.unique() < second.unique()) return -1;
    else if (first.unique() > second.unique()) return 1;
    else return 0;
}

const char *IDComparator::Name() const
{
    return "IDComparator";
}

void IDComparator::FindShortestSeparator(std::string *start, const rocksdb::Slice &limit) const
{
}

void IDComparator::FindShortSuccessor(std::string *key) const
{
}

bool IDComparator::Equal(const rocksdb::Slice &a, const rocksdb::Slice &b) const
{
    return Compare(a, b) == 0;
}
} // namespace reqstore
} // namespace hydrosphere

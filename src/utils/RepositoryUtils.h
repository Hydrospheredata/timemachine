//
// Created by Dmitry Isaev on 2019-03-07.
//

#include "reqstore_service.grpc.pb.h"
#include "rocksdb/db.h"
#include <random>

#ifndef REQSTORE_REPOSITORYUTILS_H
#define REQSTORE_REPOSITORYUTILS_H

namespace hydrosphere
{
namespace reqstore
{

namespace utils
{

class RepositoryUtils
{
public:
    static void SerializeID(const hydrosphere::reqstore::ID *, char *);
    static hydrosphere::reqstore::ID DeserializeID(const rocksdb::Slice &);
};

} // namespace utils
} // namespace reqstore
} // namespace hydrosphere

#endif //REQSTORE_REPOSITORYUTILS_H

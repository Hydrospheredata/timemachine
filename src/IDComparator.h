#include "rocksdb/db.h"
#include "utils/RepositoryUtils.h"

#ifndef REQSTORE_ID_COMPARATOR_H
#define REQSTORE_ID_COMPARATOR_H

namespace hydrosphere
{
namespace reqstore
{

class IDComparator : public rocksdb::Comparator, hydrosphere::reqstore::utils::RepositoryUtils
{
public:
    virtual int Compare(const rocksdb::Slice &a, const rocksdb::Slice &b) const override;

    virtual const char *Name() const override;

    virtual void FindShortestSeparator(std::string *start, const rocksdb::Slice &limit) const override;

    virtual void FindShortSuccessor(std::string *key) const override;

    virtual bool Equal(const rocksdb::Slice &a, const rocksdb::Slice &b) const override;
};

} // namespace reqstore
} // namespace hydrosphere

#endif

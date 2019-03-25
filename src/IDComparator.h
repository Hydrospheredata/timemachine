#include "rocksdb/db.h"
#include "utils/RepositoryUtils.h"

#ifndef TIMEMACHINE_ID_COMPARATOR_H
#define TIMEMACHINE_ID_COMPARATOR_H

namespace timemachine {

    class IDComparator : public rocksdb::Comparator, timemachine::utils::RepositoryUtils  {
    public:
        virtual int Compare(const rocksdb::Slice &a, const rocksdb::Slice &b) const override;

        virtual const char *Name() const override;

        virtual void FindShortestSeparator(std::string *start, const rocksdb::Slice &limit) const override;

        virtual void FindShortSuccessor(std::string *key) const override;

        virtual bool Equal(const rocksdb::Slice &a, const rocksdb::Slice &b) const override;

    };

}

#endif


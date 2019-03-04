#include "rocksdb/db.h"

namespace timemachine {
    
    class IDComparator : public rocksdb::Comparator {
    public:
        virtual int Compare(const rocksdb::Slice &a, const rocksdb::Slice &b) const override;

        virtual const char *Name() const override;

        virtual void FindShortestSeparator(std::string *start, const rocksdb::Slice &limit) const override;

        virtual void FindShortSuccessor(std::string *key) const override;

        virtual bool Equal(const rocksdb::Slice &a, const rocksdb::Slice &b) const override;

    };

}


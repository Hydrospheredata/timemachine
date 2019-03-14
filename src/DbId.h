//
// Created by Dmitry Isaev on 2019-03-13.
//

#ifndef TIMEMACHINE_DBID_H
#define TIMEMACHINE_DBID_H

#include "rocksdb/db.h"

public class DbId {
    public:
        DbId(uint64 _timestamp, unit64 _unique) : timestamp(_timestamp), unique(_unique): {}
        rocksdb::Slice slice() const;
    private:
        uint64 timestamp;
        unit64 unique;
};


#endif //TIMEMACHINE_DBID_H

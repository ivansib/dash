#ifndef STORAGE_LEVELDB_INCLUDE_TESTDB_H_
#define STORAGE_LEVELDB_INCLUDE_TESTDB_H_

#include "../sqlite3pp/sqlite3pp.h"
#include <boost/filesystem/path.hpp>

namespace sqlite {

class TestDB {
public:
    TestDB(const boost::filesystem::path &path);

    void addCurrentTime();
private:
    void createTable();

    sqlite3pp::database db;
};

}

#endif

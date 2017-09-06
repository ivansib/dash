#include <ctime>
#include "sqlite/testdb.h"

namespace sqlite {

TestDB::TestDB(const boost::filesystem::path &path)
{
    std::string dbFile = path.string() + "/test.db";
    db = sqlite3pp::database(dbFile.c_str());
    createTable();
    addCurrentTime();
}

void TestDB::addCurrentTime()
{
    sqlite3pp::command cmd(db, "INSERT INTO current (current_time) VALUES (?)");
    int time = static_cast<int>(std::time(nullptr));
    cmd.bind(1, time);
    cmd.execute();
}

void TestDB::createTable()
{
    db.execute("create table if not exists current (current_time INT)");
}

}

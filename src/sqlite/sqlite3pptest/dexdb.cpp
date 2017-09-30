#include <ctime>
#include "sqlite/dexdb.h"

namespace sqlite {

DexDB::DexDB(const boost::filesystem::path &path)
{
    std::string dbFile = path.string() + "/dex.db";
    db = sqlite3pp::database(dbFile.c_str());
    createTable();
    addCurrentTime();
}

void DexDB::addCurrentTime()
{
    sqlite3pp::command cmd(db, "INSERT INTO current (current_time) VALUES (?)");
    int time = static_cast<int>(std::time(nullptr));
    cmd.bind(1, time);
    cmd.execute();
}

void DexDB::createTable()
{
    db.execute("create table if not exists current (current_time INT)");
}

}

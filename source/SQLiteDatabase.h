#pragma once

#include "IDatabase.h"
#include "Options.h"

#include <any>
#include <sqlite3.h>

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::collector
{

class SQLiteDatabase : public IDatabase, public std::enable_shared_from_this<SQLiteDatabase>
{
public:
    enum class QueryType {
        Check,
        Create,
        DropTables,
        LoadDevices,
        GetDevices,
        GetSessions,
        AddDevice,
        RemDevice,
        HasDevice,
        AddSession,
        EndSession,
        CleanSessions
    };

    typedef struct Query {
        QueryType type;
        void *raw;
    } Query;

public:
    SQLiteDatabase(SQLiteDatabase const &) = delete;
    void operator=(SQLiteDatabase const &) = delete;

    void enableEvents() final;
    auto getShared() -> std::shared_ptr<SQLiteDatabase> { return shared_from_this(); }
    auto requestHandler(const IDatabase::Request &request) -> bool final;

    auto runQuery(const std::string &sql, Query &query) -> bool;

public:
    SQLiteDatabase();
    ~SQLiteDatabase();

private:
    sqlite3 *m_db = nullptr;
};

} // namespace tkm::collector

/*-
 * Copyright (c) 2020 Alin Popa
 * All rights reserved.
 */

/*
 * @author Alin Popa <alin.popa@fxdata.ro>
 */

#pragma once

#include "IDatabase.h"
#include "Options.h"

#include <any>
#include <sqlite3.h>

using namespace bswi::log;
using namespace bswi::event;

namespace cds::server
{

class SQLiteDatabase : public IDatabase, public std::enable_shared_from_this<SQLiteDatabase>
{
public:
    enum class QueryType {
        Check,
        Create,
        TokenOwnerId,
        DropTables,
        GetUserId,
        GetProjectId,
        GetWorkPackageId,
        GetUsers,
        GetProjects,
        GetWorkPackages,
        GetTokens,
        AddUser,
        RemUser,
        AddProject,
        RemProject,
        AddWorkPackage,
        RemWorkPackage,
        UpdateUserRealName,
        UpdateUserPassword,
        UpdateUserProjects,
        UpdateUserEmail,
        AddToken,
        RemToken,
        AddDltFilter,
        RemDltFilter,
        SetDltFilterActive,
        GetDltFilterId,
        GetDltFilters,
        AddSyslogFilter,
        RemSyslogFilter,
        SetSyslogFilterActive,
        GetSyslogFilterId,
        GetSyslogFilters,
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

} // namespace cds::server

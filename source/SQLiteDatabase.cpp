/*-
 * Copyright (c) 2020 Alin Popa
 * All rights reserved.
 */

/*
 * @author Alin Popa <alin.popa@fxdata.ro>
 */

#include "SQLiteDatabase.h"
#include "Application.h"
#include "Defaults.h"
#include "Query.h"
#include "Server.pb.h"

#include <Envelope.pb.h>
#include <Helpers.h>
#include <any>
#include <filesystem>
#include <string>
#include <vector>

using std::shared_ptr;
using std::string;
namespace fs = std::filesystem;

namespace cds::server
{

static auto sqlite_callback(void *data, int argc, char **argv, char **colname) -> int;
static auto doCheckDatabase(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doInitDatabase(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doAuthUser(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool;
static auto doGetUsers(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool;
static auto doGetProjects(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doGetWorkPackages(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doAddToken(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool;
static auto doRemoveToken(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doGetTokens(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool;
static auto doAddUser(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool;
static auto doUpdateUser(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doAddProject(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doRemoveUser(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doRemoveProject(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doAddWorkPackage(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doRemoveWorkPackage(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doAddDltFilter(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doRemoveDltFilter(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doSetDltFilterActive(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doGetDltFilters(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doAddSyslogFilter(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doRemoveSyslogFilter(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doSetSyslogFilterActive(const shared_ptr<SQLiteDatabase> &db,
                                    const IDatabase::Request &rq) -> bool;
static auto doGetSyslogFilters(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doConnect(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool;
static auto doDisconnect(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doGetEntries(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;

SQLiteDatabase::SQLiteDatabase()
: IDatabase()
{
    fs::path addr(CDSApp()->getOptions()->getFor(Options::Key::DBServerAddress));
    logDebug() << "Using DB file: " << addr.string();
    if (sqlite3_open(addr.c_str(), &m_db) != SQLITE_OK) {
        sqlite3_close(m_db);
        throw std::runtime_error(sqlite3_errmsg(m_db));
    }
}

SQLiteDatabase::~SQLiteDatabase()
{
    sqlite3_close(m_db);
}

void SQLiteDatabase::enableEvents()
{
    CDSApp()->addEventSource(m_queue);

    IDatabase::Request dbrq {.action = IDatabase::Action::CheckDatabase};
    pushRequest(dbrq);
}

auto SQLiteDatabase::runQuery(const std::string &sql, SQLiteDatabase::Query &query) -> bool
{
    char *queryError = nullptr;

    logDebug() << "Run query: " << sql;
    if (::sqlite3_exec(m_db, sql.c_str(), sqlite_callback, &query, &queryError) != SQLITE_OK) {
        logError() << "SQLiteDatabase query error: " << queryError;
        sqlite3_free(queryError);
        return false;
    }

    return true;
}

static auto sqlite_callback(void *data, int argc, char **argv, char **colname) -> int
{
    auto *query = static_cast<SQLiteDatabase::Query *>(data);

    switch (query->type) {
    case SQLiteDatabase::QueryType::Check:
    case SQLiteDatabase::QueryType::Create:
    case SQLiteDatabase::QueryType::DropTables:
    case SQLiteDatabase::QueryType::UpdateUserRealName:
    case SQLiteDatabase::QueryType::UpdateUserPassword:
    case SQLiteDatabase::QueryType::UpdateUserProjects:
    case SQLiteDatabase::QueryType::UpdateUserEmail:
    case SQLiteDatabase::QueryType::AddUser:
    case SQLiteDatabase::QueryType::RemUser:
    case SQLiteDatabase::QueryType::AddProject:
    case SQLiteDatabase::QueryType::RemProject:
    case SQLiteDatabase::QueryType::AddWorkPackage:
    case SQLiteDatabase::QueryType::RemWorkPackage:
    case SQLiteDatabase::QueryType::AddToken:
    case SQLiteDatabase::QueryType::RemToken:
    case SQLiteDatabase::QueryType::AddDltFilter:
    case SQLiteDatabase::QueryType::RemDltFilter:
    case SQLiteDatabase::QueryType::SetDltFilterActive:
    case SQLiteDatabase::QueryType::AddSyslogFilter:
    case SQLiteDatabase::QueryType::RemSyslogFilter:
    case SQLiteDatabase::QueryType::SetSyslogFilterActive:
        break;
    case SQLiteDatabase::QueryType::GetUsers: {
        auto pld = static_cast<std::vector<cds::msg::server::UserData> *>(query->raw);
        cds::msg::server::UserData user {};
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i], cdsQuery.m_userColumn.at(Query::UserColumn::Name).c_str(), 60)
                == 0) {
                user.set_name(argv[i]);
            } else if (strncmp(colname[i],
                               cdsQuery.m_userColumn.at(Query::UserColumn::RealName).c_str(),
                               60)
                       == 0) {
                user.set_realname(argv[i]);
            } else if (strncmp(colname[i],
                               cdsQuery.m_userColumn.at(Query::UserColumn::Password).c_str(),
                               60)
                       == 0) {
                user.set_password(argv[i]);
            } else if (strncmp(colname[i],
                               cdsQuery.m_userColumn.at(Query::UserColumn::Email).c_str(),
                               60)
                       == 0) {
                user.set_email(argv[i]);
            } else if (strncmp(colname[i],
                               cdsQuery.m_userColumn.at(Query::UserColumn::Projects).c_str(),
                               60)
                       == 0) {
                user.set_projects(argv[i]);
            } else if (strncmp(
                           colname[i], cdsQuery.m_userColumn.at(Query::UserColumn::Id).c_str(), 60)
                       == 0) {
                user.set_id(std::stoi(argv[i]));
            }
        }
        pld->emplace_back(user);
        break;
    }
    case SQLiteDatabase::QueryType::GetProjects: {
        auto pld = static_cast<std::vector<cds::msg::server::ProjectData> *>(query->raw);
        cds::msg::server::ProjectData proj {};
        for (int i = 0; i < argc; i++) {
            if (strncmp(
                    colname[i], cdsQuery.m_projectColumn.at(Query::ProjectColumn::Name).c_str(), 60)
                == 0) {
                proj.set_name(argv[i]);
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_projectColumn.at(Query::ProjectColumn::Description).c_str(),
                           60)
                       == 0) {
                proj.set_description(argv[i]);
            } else if (strncmp(colname[i],
                               cdsQuery.m_projectColumn.at(Query::ProjectColumn::Owner).c_str(),
                               60)
                       == 0) {
                proj.set_owner(argv[i]);
            } else if (strncmp(colname[i],
                               cdsQuery.m_projectColumn.at(Query::ProjectColumn::Id).c_str(),
                               60)
                       == 0) {
                proj.set_id(std::stoi(argv[i]));
            }
        }
        pld->emplace_back(proj);
        break;
    }
    case SQLiteDatabase::QueryType::GetWorkPackages: {
        auto pld = static_cast<std::vector<cds::msg::server::WorkPackageData> *>(query->raw);
        cds::msg::server::WorkPackageData wp {};
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i],
                        cdsQuery.m_workPackageColumn.at(Query::WorkPackageColumn::Name).c_str(),
                        60)
                == 0) {
                wp.set_name(argv[i]);
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_workPackageColumn.at(Query::WorkPackageColumn::Type).c_str(),
                           60)
                       == 0) {
                wp.set_type(
                    static_cast<cds::msg::server::WorkPackageData::LogSystem>(std::stoi(argv[i])));
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_workPackageColumn.at(Query::WorkPackageColumn::Owner).c_str(),
                           60)
                       == 0) {
                wp.set_owner(argv[i]);
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_workPackageColumn.at(Query::WorkPackageColumn::Id).c_str(),
                           60)
                       == 0) {
                wp.set_id(std::stoi(argv[i]));
            } else if (strncmp(colname[i],
                               cdsQuery.m_workPackageColumn.at(Query::WorkPackageColumn::Project)
                                   .c_str(),
                               60)
                       == 0) {
                wp.set_project(argv[i]);
            }
        }
        pld->emplace_back(wp);
        break;
    }
    case SQLiteDatabase::QueryType::GetTokens: {
        auto pld = static_cast<std::vector<cds::msg::server::AccessToken> *>(query->raw);
        cds::msg::server::AccessToken accessToken {};
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i],
                        cdsQuery.m_accessTokenColumn.at(Query::AccessTokenColumn::Name).c_str(),
                        60)
                == 0) {
                accessToken.set_name(argv[i]);
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_accessTokenColumn.at(Query::AccessTokenColumn::Description)
                               .c_str(),
                           60)
                       == 0) {
                accessToken.set_description(argv[i]);
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_accessTokenColumn.at(Query::AccessTokenColumn::Owner).c_str(),
                           60)
                       == 0) {
                accessToken.set_owner(argv[i]);
            } else if (strncmp(colname[i],
                               cdsQuery.m_accessTokenColumn.at(Query::AccessTokenColumn::Timestamp)
                                   .c_str(),
                               60)
                       == 0) {
                accessToken.set_timestamp(std::stoull(argv[i]));
            }
        }
        pld->emplace_back(accessToken);
        break;
    }
    case SQLiteDatabase::QueryType::GetDltFilters: {
        auto pld = static_cast<std::vector<cds::msg::server::DltFilter> *>(query->raw);
        cds::msg::server::DltFilter dltFilter {};
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i],
                        cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::Id).c_str(),
                        60)
                == 0) {
                dltFilter.set_id(std::stol(argv[i]));
            } else if (strncmp(colname[i],
                               cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::Name).c_str(),
                               60)
                       == 0) {
                dltFilter.set_name(argv[i]);
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::Active).c_str(),
                           60)
                       == 0) {
                dltFilter.set_active(static_cast<bool>(std::stoi(argv[i])));
            } else if (strncmp(colname[i],
                               cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::Type).c_str(),
                               60)
                       == 0) {
                dltFilter.set_type(
                    static_cast<cds::msg::server::DltFilter::Type>(std::stoi(argv[i])));
            } else if (strncmp(colname[i],
                               cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::Color).c_str(),
                               60)
                       == 0) {
                dltFilter.set_color(
                    static_cast<cds::msg::server::DltEntry::Color>(std::stoi(argv[i])));
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::MsgType).c_str(),
                           60)
                       == 0) {
                dltFilter.set_msgtype(
                    static_cast<cds::msg::server::DltEntry::Type>(std::stoi(argv[i])));
            } else if (strncmp(colname[i],
                               cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::MsgSubType)
                                   .c_str(),
                               60)
                       == 0) {
                dltFilter.set_msgsubtype(
                    static_cast<cds::msg::server::DltEntry::SubType>(std::stoi(argv[i])));
            } else if (strncmp(colname[i],
                               cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::EcuID).c_str(),
                               60)
                       == 0) {
                dltFilter.set_ecuid(argv[i]);
            } else if (strncmp(colname[i],
                               cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::AppID).c_str(),
                               60)
                       == 0) {
                dltFilter.set_appid(argv[i]);
            } else if (strncmp(colname[i],
                               cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::CtxID).c_str(),
                               60)
                       == 0) {
                dltFilter.set_ctxid(argv[i]);
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::Payload).c_str(),
                           60)
                       == 0) {
                dltFilter.set_payload(argv[i]);
            }
        }
        pld->emplace_back(dltFilter);
        break;
    }
    case SQLiteDatabase::QueryType::GetSyslogFilters: {
        auto pld = static_cast<std::vector<cds::msg::server::SyslogFilter> *>(query->raw);
        cds::msg::server::SyslogFilter slgFilter {};
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i],
                        cdsQuery.m_slgFilterColumn.at(Query::SLGFilterColumn::Id).c_str(),
                        60)
                == 0) {
                slgFilter.set_id(std::stol(argv[i]));
            } else if (strncmp(colname[i],
                               cdsQuery.m_slgFilterColumn.at(Query::SLGFilterColumn::Name).c_str(),
                               60)
                       == 0) {
                slgFilter.set_name(argv[i]);
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_slgFilterColumn.at(Query::SLGFilterColumn::Active).c_str(),
                           60)
                       == 0) {
                slgFilter.set_active(static_cast<bool>(std::stoi(argv[i])));
            } else if (strncmp(colname[i],
                               cdsQuery.m_slgFilterColumn.at(Query::SLGFilterColumn::Type).c_str(),
                               60)
                       == 0) {
                slgFilter.set_type(
                    static_cast<cds::msg::server::SyslogFilter::Type>(std::stoi(argv[i])));
            } else if (strncmp(colname[i],
                               cdsQuery.m_slgFilterColumn.at(Query::SLGFilterColumn::Color).c_str(),
                               60)
                       == 0) {
                slgFilter.set_color(
                    static_cast<cds::msg::server::SyslogEntry::Color>(std::stoi(argv[i])));
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_slgFilterColumn.at(Query::SLGFilterColumn::Priority).c_str(),
                           60)
                       == 0) {
                slgFilter.set_priority(
                    static_cast<cds::msg::server::SyslogEntry::Priority>(std::stoi(argv[i])));
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_slgFilterColumn.at(Query::SLGFilterColumn::Hostname).c_str(),
                           60)
                       == 0) {
                slgFilter.set_hostname(argv[i]);
            } else if (strncmp(
                           colname[i],
                           cdsQuery.m_slgFilterColumn.at(Query::SLGFilterColumn::Payload).c_str(),
                           60)
                       == 0) {
                slgFilter.set_payload(argv[i]);
            }
        }
        pld->emplace_back(slgFilter);
        break;
    }
    case SQLiteDatabase::QueryType::TokenOwnerId: {
        auto pld = static_cast<int *>(query->raw);
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i],
                        cdsQuery.m_accessTokenColumn.at(Query::AccessTokenColumn::Owner).c_str(),
                        60)
                == 0) {
                *pld = std::stol(argv[i]);
            }
        }
        break;
    }
    case SQLiteDatabase::QueryType::GetUserId: {
        auto pld = static_cast<int *>(query->raw);
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i], cdsQuery.m_userColumn.at(Query::UserColumn::Id).c_str(), 60)
                == 0) {
                *pld = std::stol(argv[i]);
            }
        }
        break;
    }
    case SQLiteDatabase::QueryType::GetProjectId: {
        auto pld = static_cast<int *>(query->raw);
        for (int i = 0; i < argc; i++) {
            if (strncmp(
                    colname[i], cdsQuery.m_projectColumn.at(Query::ProjectColumn::Id).c_str(), 60)
                == 0) {
                *pld = std::stol(argv[i]);
            }
        }
        break;
    }
    case SQLiteDatabase::QueryType::GetWorkPackageId: {
        auto pld = static_cast<int *>(query->raw);
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i],
                        cdsQuery.m_workPackageColumn.at(Query::WorkPackageColumn::Id).c_str(),
                        60)
                == 0) {
                *pld = std::stol(argv[i]);
            }
        }
        break;
    }
    case SQLiteDatabase::QueryType::GetDltFilterId: {
        auto pld = static_cast<int *>(query->raw);
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i],
                        cdsQuery.m_dltFilterColumn.at(Query::DLTFilterColumn::Id).c_str(),
                        60)
                == 0) {
                *pld = std::stol(argv[i]);
            }
        }
        break;
    }
    case SQLiteDatabase::QueryType::GetSyslogFilterId: {
        auto pld = static_cast<int *>(query->raw);
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i],
                        cdsQuery.m_slgFilterColumn.at(Query::SLGFilterColumn::Id).c_str(),
                        60)
                == 0) {
                *pld = std::stol(argv[i]);
            }
        }
        break;
    }
    default:
        logError() << "Unknown query type";
        break;
    }

    return 0;
}

auto SQLiteDatabase::requestHandler(const Request &rq) -> bool
{
    switch (rq.action) {
    case IDatabase::Action::CheckDatabase:
        return doCheckDatabase(getShared(), rq);
    case IDatabase::Action::InitDatabase:
        return doInitDatabase(getShared(), rq);
    case IDatabase::Action::AuthUser:
        return doAuthUser(getShared(), rq);
    case IDatabase::Action::Connect:
        return doConnect(getShared(), rq);
    case IDatabase::Action::Disconnect:
        return doDisconnect(getShared(), rq);
    case IDatabase::Action::GetEntries:
        return doGetEntries(getShared(), rq);
    case IDatabase::Action::GetUsers:
        return doGetUsers(getShared(), rq);
    case IDatabase::Action::GetProjects:
        return doGetProjects(getShared(), rq);
    case IDatabase::Action::GetWorkPackages:
        return doGetWorkPackages(getShared(), rq);
    case IDatabase::Action::GetTokens:
        return doGetTokens(getShared(), rq);
    case IDatabase::Action::AddToken:
        return doAddToken(getShared(), rq);
    case IDatabase::Action::RemoveToken:
        return doRemoveToken(getShared(), rq);
    case IDatabase::Action::AddUser:
        return doAddUser(getShared(), rq);
    case IDatabase::Action::UpdateUser:
        return doUpdateUser(getShared(), rq);
    case IDatabase::Action::AddProject:
        return doAddProject(getShared(), rq);
    case IDatabase::Action::RemoveUser:
        return doRemoveUser(getShared(), rq);
    case IDatabase::Action::RemoveProject:
        return doRemoveProject(getShared(), rq);
    case IDatabase::Action::AddWorkPackage:
        return doAddWorkPackage(getShared(), rq);
    case IDatabase::Action::RemoveWorkPackage:
        return doRemoveWorkPackage(getShared(), rq);
    case IDatabase::Action::AddDltFilter:
        return doAddDltFilter(getShared(), rq);
    case IDatabase::Action::RemoveDltFilter:
        return doRemoveDltFilter(getShared(), rq);
    case IDatabase::Action::SetDltFilterActive:
        return doSetDltFilterActive(getShared(), rq);
    case IDatabase::Action::GetDltFilters:
        return doGetDltFilters(getShared(), rq);
    case IDatabase::Action::AddSyslogFilter:
        return doAddSyslogFilter(getShared(), rq);
    case IDatabase::Action::RemoveSyslogFilter:
        return doRemoveSyslogFilter(getShared(), rq);
    case IDatabase::Action::SetSyslogFilterActive:
        return doSetSyslogFilterActive(getShared(), rq);
    case IDatabase::Action::GetSyslogFilters:
        return doGetSyslogFilters(getShared(), rq);
    default:
        break;
    }
    logError() << "Unknown action request";
    return false;
}

static auto doCheckDatabase(const shared_ptr<SQLiteDatabase> &db, const SQLiteDatabase::Request &rq)
    -> bool
{
    auto userId = -1;

    logDebug() << "Handling DB check request";

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;
    auto status
        = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, "admin"), queryCheckExisting);

    if (!status || (userId == -1)) {
        SQLiteDatabase::Query dropTablesQuery {.type = SQLiteDatabase::QueryType::DropTables};
        status = db->runQuery(cdsQuery.dropTables(Query::Type::SQLite3), dropTablesQuery);

        if (status) {
            SQLiteDatabase::Query createTablesQuery {.type = SQLiteDatabase::QueryType::Create};
            status = db->runQuery(cdsQuery.createTables(Query::Type::SQLite3), createTablesQuery);
        }

        if (status) {
            SQLiteDatabase::Query addUserQuery {.type = SQLiteDatabase::QueryType::AddUser};
            status = db->runQuery(cdsQuery.addUser(Query::Type::SQLite3,
                                                   "admin",
                                                   helpers::pskHsh("admin"),
                                                   "CDS Administrator",
                                                   "admin@localhost",
                                                   "na"),
                                  addUserQuery);
        }
    }

    return status;
}

static auto doInitDatabase(const shared_ptr<SQLiteDatabase> &db, const SQLiteDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    logDebug() << "Handling DB init request";

    if (rq.args.count(Defaults::Arg::Forced)) {
        if (rq.args.at(Defaults::Arg::Forced) == cdsDefaults.valFor(Defaults::Val::True)) {
            SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::DropTables};
            db->runQuery(cdsQuery.dropTables(Query::Type::SQLite3), query);
        }
    }

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::Create};
    auto status = db->runQuery(cdsQuery.createTables(Query::Type::SQLite3), query);
    if (status) {
        SQLiteDatabase::Query addUserQuery {.type = SQLiteDatabase::QueryType::AddUser};
        status = db->runQuery(cdsQuery.addUser(Query::Type::SQLite3,
                                               "admin",
                                               helpers::pskHsh("admin"),
                                               "CDS Administrator",
                                               "admin@localhost",
                                               "na"),
                              addUserQuery);
    }

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }
    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason,
                     status == true ? "Database init complete"
                                    : "Database init failed. Query error");

    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doGetUsers(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB GetUsers request from client: " << rq.client->getName();

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::GetUsers};
    auto queryUserList = std::vector<cds::msg::server::UserData>();
    query.raw = &queryUserList;

    auto status = db->runQuery(cdsQuery.getUsers(Query::Type::SQLite3), query);
    if (status) {
        cds::msg::Envelope envelope;
        cds::msg::server::Message message;
        cds::msg::server::UserList userList;

        for (auto &user : queryUserList) {
            auto tmpUser = userList.add_user();
            tmpUser->CopyFrom(user);
        }

        // As response to client registration request we ask client to send descriptor
        message.set_type(msg::server::Message_Type_UserList);
        message.mutable_data()->PackFrom(userList);
        envelope.mutable_mesg()->PackFrom(message);

        envelope.set_target(msg::Envelope_Recipient_Any);
        envelope.set_origin(msg::Envelope_Recipient_Server);

        if (!rq.client->writeEnvelope(envelope)) {
            logWarn() << "Fail to send user list to client " << rq.client->getFD();
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to send user list");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "List provided");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Query failed");
        logError() << "Query error for getUsers";
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));

    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doGetProjects(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB GetProjects request from client: " << rq.client->getName();

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::GetProjects};
    auto queryProjectList = std::vector<cds::msg::server::ProjectData>();
    query.raw = &queryProjectList;

    auto status = db->runQuery(cdsQuery.getProjects(Query::Type::SQLite3), query);
    if (status) {
        cds::msg::Envelope envelope;
        cds::msg::server::Message message;
        cds::msg::server::ProjectList projectList;

        for (auto &proj : queryProjectList) {
            auto tmpProj = projectList.add_project();
            tmpProj->CopyFrom(proj);
        }

        // As response to client registration request we ask client to send descriptor
        message.set_type(msg::server::Message_Type_ProjectList);
        message.mutable_data()->PackFrom(projectList);
        envelope.mutable_mesg()->PackFrom(message);

        envelope.set_target(msg::Envelope_Recipient_Any);
        envelope.set_origin(msg::Envelope_Recipient_Server);

        if (!rq.client->writeEnvelope(envelope)) {
            logWarn() << "Fail to send project list to client " << rq.client->getFD();
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to send project list");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "Project list provided");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Query failed");
        logError() << "Query error for getProjects";
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));

    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doGetWorkPackages(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB GetWorkPackages request from client: " << rq.client->getName();

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::GetWorkPackages};
    auto queryWPList = std::vector<cds::msg::server::WorkPackageData>();
    query.raw = &queryWPList;

    auto status = db->runQuery(cdsQuery.getWorkPackages(Query::Type::SQLite3), query);
    if (status) {
        cds::msg::Envelope envelope;
        cds::msg::server::Message message;
        cds::msg::server::WorkPackageList wpList;

        for (auto &wp : queryWPList) {
            auto tmpWP = wpList.add_workpackage();
            tmpWP->CopyFrom(wp);
        }

        // As response to client registration request we ask client to send descriptor
        message.set_type(msg::server::Message_Type_WorkPackageList);
        message.mutable_data()->PackFrom(wpList);
        envelope.mutable_mesg()->PackFrom(message);

        envelope.set_target(msg::Envelope_Recipient_Any);
        envelope.set_origin(msg::Envelope_Recipient_Server);

        if (!rq.client->writeEnvelope(envelope)) {
            logWarn() << "Fail to send work package list to client " << rq.client->getFD();
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to send work package list");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "Work package list provided");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Query failed");
        logError() << "Query error for getWorkPackages";
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));

    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doGetTokens(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB GetTokens request from client: " << rq.client->getName();
    const auto &userData = std::any_cast<cds::msg::server::UserData>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;
    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, userData.name()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::GetTokens};
        auto queryATList = std::vector<cds::msg::server::AccessToken>();
        query.raw = &queryATList;

        status
            = db->runQuery(cdsQuery.getAccessTokens(Query::Type::SQLite3, userData.name()), query);
        if (!status) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to get access tokens");
        } else {
            cds::msg::Envelope envelope;
            cds::msg::server::Message message;
            cds::msg::server::AccessTokenList atList;

            for (auto &accessToken : queryATList) {
                auto tmpAT = atList.add_token();
                tmpAT->CopyFrom(accessToken);
            }

            // As response to client registration request we ask client to send descriptor
            message.set_type(msg::server::Message_Type_AccessTokenList);
            message.mutable_data()->PackFrom(atList);
            envelope.mutable_mesg()->PackFrom(message);

            envelope.set_target(msg::Envelope_Recipient_Viewer);
            envelope.set_origin(msg::Envelope_Recipient_Server);

            status = rq.client->writeEnvelope(envelope);
            if (!status) {
                logWarn() << "Fail to send access tokeni list to client " << rq.client->getFD();
                mrq.args.emplace(Defaults::Arg::Reason, "Failed to send access token list");
            } else {
                mrq.args.emplace(Defaults::Arg::Reason, "Access token list provided");
            }
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doAuthUser(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::AuthStatus};
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB AuthUser request from client: " << rq.client->getName();
    const auto &userData = std::any_cast<cds::msg::server::UserData>(rq.bulkData);

    // We set the owner for AuthStatus
    mrq.args.emplace(Defaults::Arg::Owner, userData.name());

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;
    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, userData.name()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "User not found");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
        userId = -1; // reset user ID for the next query

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::GetUserId};
        query.raw = &userId;

        status = db->runQuery(
            cdsQuery.authUser(Query::Type::SQLite3, userData.name(), userData.password()), query);

        // If not found run the query for access tokens
        if (status && (userId == -1)) {
            query.type = SQLiteDatabase::QueryType::TokenOwnerId;
            status = db->runQuery(
                cdsQuery.authUser(Query::Type::SQLite3, userData.name(), userData.password(), true),
                query);
        }

        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        }

        if (!status || (userId == -1)) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to authentificate user");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "User authentificated");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doAddUser(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB AddUser request from client: " << rq.client->getName();
    const auto &userData = std::any_cast<cds::msg::server::UserData>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;
    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, userData.name()),
                               queryCheckExisting);
    if (status) {
        if (rq.args.count(Defaults::Arg::Forced)) {
            if (rq.args.at(Defaults::Arg::Forced) == cdsDefaults.valFor(Defaults::Val::True)) {
                SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::RemUser};
                db->runQuery(cdsQuery.remUser(Query::Type::SQLite3, userData.name()), query);
            }
        } else if (userId != -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "User already exists");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::AddUser};
        status = db->runQuery(cdsQuery.addUser(Query::Type::SQLite3,
                                               userData.name(),
                                               userData.password(),
                                               userData.realname(),
                                               userData.email(),
                                               userData.projects()),
                              query);
        if (!status) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to add user");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "User added");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doUpdateUser(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    std::stringstream reasonMesage;
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB UpdateUser request from client: " << rq.client->getName();

    const auto &checkUserData = std::any_cast<cds::msg::server::UserData>(rq.bulkData);
    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;
    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, checkUserData.name()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "User does not exist");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }

        const auto &userData = std::any_cast<cds::msg::server::UserData>(rq.bulkData);

        if (!userData.realname().empty()) {
            SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::UpdateUserRealName};
            status = db->runQuery(cdsQuery.updUserRealName(
                                      Query::Type::SQLite3, userData.name(), userData.realname()),
                                  query);
            if (!status) {
                reasonMesage << "Failed to update user real name. ";
            } else {
                reasonMesage << "User real name updated. ";
            }
        }

        if (!userData.password().empty()) {
            SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::UpdateUserPassword};
            status = db->runQuery(cdsQuery.updUserPassword(
                                      Query::Type::SQLite3, userData.name(), userData.password()),
                                  query);
            if (!status) {
                reasonMesage << "Failed to update user password. ";
            } else {
                reasonMesage << "User password updated. ";
            }
        }

        if (!userData.projects().empty()) {
            SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::UpdateUserProjects};
            status = db->runQuery(cdsQuery.updUserProjects(
                                      Query::Type::SQLite3, userData.name(), userData.projects()),
                                  query);
            if (!status) {
                reasonMesage << "Failed to update user projects. ";
            } else {
                reasonMesage << "User projects updated. ";
            }
        }

        if (!userData.email().empty()) {
            SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::UpdateUserEmail};
            status = db->runQuery(
                cdsQuery.updUserEmail(Query::Type::SQLite3, userData.name(), userData.email()),
                query);
            if (!status) {
                reasonMesage << "Failed to update user email. ";
            } else {
                reasonMesage << "User email updated. ";
            }
        }

        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusOkay));
        mrq.args.emplace(Defaults::Arg::Reason, reasonMesage.str());
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doAddProject(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto projId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB AddProject request from client: " << rq.client->getName();
    const auto &projData = std::any_cast<cds::msg::server::ProjectData>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetProjectId};
    queryCheckExisting.raw = &projId;
    auto status = db->runQuery(cdsQuery.getProjectId(Query::Type::SQLite3, projData.name()),
                               queryCheckExisting);
    if (status) {
        if (projId != -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "Project already exists");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::AddProject};
        status = db->runQuery(
            cdsQuery.addProject(
                Query::Type::SQLite3, projData.name(), projData.description(), projData.owner()),
            query);
        if (!status) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to add project");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "Project added");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing project");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doRemoveUser(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB RemoveUser request from client: " << rq.client->getName();

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;

    auto status
        = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, rq.args.at(Defaults::Arg::Name)),
                       queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
        }

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::RemUser};
        status = db->runQuery(
            cdsQuery.remUser(Query::Type::SQLite3, rq.args.at(Defaults::Arg::Name)), query);

        if (!status) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to remove user");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "User removed");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doRemoveProject(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto projId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB RemoveProject request from client: " << rq.client->getName();

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetProjectId};
    queryCheckExisting.raw = &projId;

    auto status
        = db->runQuery(cdsQuery.getProjectId(Query::Type::SQLite3, rq.args.at(Defaults::Arg::Name)),
                       queryCheckExisting);
    if (status) {
        if (projId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such project");
        }

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::RemProject};
        status = db->runQuery(
            cdsQuery.remProject(Query::Type::SQLite3, rq.args.at(Defaults::Arg::Name)), query);
        if (!status) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to remove project");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "Project removed");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing project");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doAddWorkPackage(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto wpId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB AddWorkPackage request from client: " << rq.client->getName();
    const auto &wpData = std::any_cast<cds::msg::server::WorkPackageData>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetWorkPackageId};
    queryCheckExisting.raw = &wpId;
    auto status = db->runQuery(cdsQuery.getWorkPackageId(Query::Type::SQLite3, wpData.name()),
                               queryCheckExisting);
    if (status) {
        if (wpId != -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "WorkPackage already exists");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::AddProject};
        status = db->runQuery(cdsQuery.addWorkPackage(Query::Type::SQLite3,
                                                      wpData.name(),
                                                      wpData.type(),
                                                      wpData.project(),
                                                      wpData.owner()),
                              query);
        if (!status) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to add work package");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "Work package added");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing work package");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doRemoveWorkPackage(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto wpId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB RemoveWorkPackage request from client: " << rq.client->getName();

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetWorkPackageId};
    queryCheckExisting.raw = &wpId;

    auto status = db->runQuery(
        cdsQuery.getWorkPackageId(Query::Type::SQLite3, rq.args.at(Defaults::Arg::Name)),
        queryCheckExisting);
    if (status) {
        if (wpId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such work package");
        }

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::RemWorkPackage};
        status = db->runQuery(
            cdsQuery.remWorkPackage(Query::Type::SQLite3, rq.args.at(Defaults::Arg::Name)), query);
        if (!status) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to remove work package");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "Work package removed");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing work package");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doAddToken(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB AddToken request from client: " << rq.client->getName();
    const auto &accessToken = std::any_cast<cds::msg::server::AccessToken>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;
    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, accessToken.owner()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::AddToken};
        status = db->runQuery(cdsQuery.addAccessToken(Query::Type::SQLite3,
                                                      accessToken.name(),
                                                      accessToken.description(),
                                                      accessToken.timestamp(),
                                                      accessToken.token(),
                                                      accessToken.owner()),
                              query);
        if (!status) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to add access token");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "Access token added");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doRemoveToken(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB RemoveToken request from client: " << rq.client->getName();
    const auto &accessToken = std::any_cast<cds::msg::server::AccessToken>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;
    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, accessToken.owner()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::RemToken};
        status = db->runQuery(
            cdsQuery.remAccessToken(Query::Type::SQLite3, accessToken.name(), accessToken.owner()),
            query);
        if (!status) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to remove access token");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "Access token removed");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doAddDltFilter(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto filterId = -1;
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB AddDltFilter request from client: " << rq.client->getName();
    const auto &dltFilter = std::any_cast<cds::msg::server::DltFilter>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;

    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, dltFilter.owner()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    queryCheckExisting.type = SQLiteDatabase::QueryType::GetDltFilterId;
    queryCheckExisting.raw = &filterId;

    status = db->runQuery(
        cdsQuery.getDltFilterId(Query::Type::SQLite3, dltFilter.name(), dltFilter.owner()),
        queryCheckExisting);
    if (status) {
        if (filterId != -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "Filter already exist");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing filter");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::AddDltFilter};
    status = db->runQuery(cdsQuery.addDltFilter(Query::Type::SQLite3,
                                                dltFilter.name(),
                                                dltFilter.owner(),
                                                dltFilter.active(),
                                                static_cast<int>(dltFilter.type()),
                                                static_cast<int>(dltFilter.color()),
                                                static_cast<int>(dltFilter.msgtype()),
                                                static_cast<int>(dltFilter.msgsubtype()),
                                                dltFilter.ecuid(),
                                                dltFilter.appid(),
                                                dltFilter.ctxid(),
                                                dltFilter.payload()),
                          query);
    if (!status) {
        mrq.args.emplace(Defaults::Arg::Reason, "Failed to add DLT filter");
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "DLT filter added");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doRemoveDltFilter(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto filterId = -1;
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB RemoveDltFilter request from client: " << rq.client->getName();
    const auto &dltFilter = std::any_cast<cds::msg::server::DltFilter>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;

    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, dltFilter.owner()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    queryCheckExisting.type = SQLiteDatabase::QueryType::GetDltFilterId;
    queryCheckExisting.raw = &filterId;

    status = db->runQuery(
        cdsQuery.getDltFilterId(Query::Type::SQLite3, dltFilter.name(), dltFilter.owner()),
        queryCheckExisting);
    if (status) {
        if (filterId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such filter");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing filter");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::RemDltFilter};
    status = db->runQuery(
        cdsQuery.remDltFilter(Query::Type::SQLite3, dltFilter.name(), dltFilter.owner()), query);
    if (!status) {
        mrq.args.emplace(Defaults::Arg::Reason, "Failed to remove DLT filter");
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "DLT filter removed");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doSetDltFilterActive(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto filterId = -1;
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB SetDltFilterActive request from client: " << rq.client->getName();
    const auto &dltFilter = std::any_cast<cds::msg::server::DltFilter>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;

    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, dltFilter.owner()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    queryCheckExisting.type = SQLiteDatabase::QueryType::GetDltFilterId;
    queryCheckExisting.raw = &filterId;

    status = db->runQuery(
        cdsQuery.getDltFilterId(Query::Type::SQLite3, dltFilter.name(), dltFilter.owner()),
        queryCheckExisting);
    if (status) {
        if (filterId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such filter");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing filter");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::SetDltFilterActive};
    status = db->runQuery(
        cdsQuery.setDltFilterActive(
            Query::Type::SQLite3, dltFilter.name(), dltFilter.owner(), dltFilter.active()),
        query);
    if (!status) {
        mrq.args.emplace(Defaults::Arg::Reason, "Failed to set DLT filter active state");
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "DLT filter active state set");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doGetDltFilters(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB GetDltFilters request from client: " << rq.client->getName();
    const auto &userData = std::any_cast<cds::msg::server::UserData>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;
    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, userData.name()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing filter");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::GetDltFilters};
    auto queryDltFilters = std::vector<cds::msg::server::DltFilter>();
    query.raw = &queryDltFilters;

    status = db->runQuery(cdsQuery.getDltFilters(Query::Type::SQLite3, userData.name()), query);
    if (!status) {
        mrq.args.emplace(Defaults::Arg::Reason, "Failed to get DLT filters");
    } else {
        cds::msg::Envelope envelope;
        cds::msg::server::Message message;
        cds::msg::server::DltFilters dltFilters;

        for (auto &dltFilter : queryDltFilters) {
            auto tmpFilter = dltFilters.add_filter();
            tmpFilter->CopyFrom(dltFilter);
        }

        message.set_type(msg::server::Message_Type_DltFilters);
        message.mutable_data()->PackFrom(dltFilters);
        envelope.mutable_mesg()->PackFrom(message);

        envelope.set_target(msg::Envelope_Recipient_Viewer);
        envelope.set_origin(msg::Envelope_Recipient_Server);

        status = rq.client->writeEnvelope(envelope);
        if (!status) {
            logWarn() << "Fail to send DLT filters to client " << rq.client->getFD();
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to send DLT filters");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "DLT filters provided");
        }
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doAddSyslogFilter(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto filterId = -1;
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB AddSyslogFilter request from client: " << rq.client->getName();
    const auto &slgFilter = std::any_cast<cds::msg::server::SyslogFilter>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;

    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, slgFilter.owner()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    queryCheckExisting.type = SQLiteDatabase::QueryType::GetSyslogFilterId;
    queryCheckExisting.raw = &filterId;

    status = db->runQuery(
        cdsQuery.getSyslogFilterId(Query::Type::SQLite3, slgFilter.name(), slgFilter.owner()),
        queryCheckExisting);
    if (status) {
        if (filterId != -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "Filter already exist");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing filter");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::AddSyslogFilter};
    status = db->runQuery(cdsQuery.addSyslogFilter(Query::Type::SQLite3,
                                                   slgFilter.name(),
                                                   slgFilter.owner(),
                                                   slgFilter.active(),
                                                   static_cast<int>(slgFilter.type()),
                                                   static_cast<int>(slgFilter.color()),
                                                   static_cast<int>(slgFilter.priority()),
                                                   slgFilter.hostname(),
                                                   slgFilter.payload()),
                          query);
    if (!status) {
        mrq.args.emplace(Defaults::Arg::Reason, "Failed to add Syslog filter");
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Syslog filter added");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doRemoveSyslogFilter(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto filterId = -1;
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB RemoveSyslogFilter request from client: " << rq.client->getName();
    const auto &slgFilter = std::any_cast<cds::msg::server::SyslogFilter>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;

    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, slgFilter.owner()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    queryCheckExisting.type = SQLiteDatabase::QueryType::GetSyslogFilterId;
    queryCheckExisting.raw = &filterId;

    status = db->runQuery(
        cdsQuery.getSyslogFilterId(Query::Type::SQLite3, slgFilter.name(), slgFilter.owner()),
        queryCheckExisting);
    if (status) {
        if (filterId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such filter");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing filter");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::RemSyslogFilter};
    status = db->runQuery(
        cdsQuery.remSyslogFilter(Query::Type::SQLite3, slgFilter.name(), slgFilter.owner()), query);
    if (!status) {
        mrq.args.emplace(Defaults::Arg::Reason, "Failed to remove Syslog filter");
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Syslog filter removed");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doSetSyslogFilterActive(const shared_ptr<SQLiteDatabase> &db,
                                    const IDatabase::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto filterId = -1;
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB SetSyslogFilterActive request from client: " << rq.client->getName();
    const auto &slgFilter = std::any_cast<cds::msg::server::SyslogFilter>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;

    auto status = db->runQuery(cdsQuery.getProjectId(Query::Type::SQLite3, slgFilter.owner()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    queryCheckExisting.type = SQLiteDatabase::QueryType::GetSyslogFilterId;
    queryCheckExisting.raw = &filterId;

    status = db->runQuery(
        cdsQuery.getSyslogFilterId(Query::Type::SQLite3, slgFilter.name(), slgFilter.owner()),
        queryCheckExisting);
    if (status) {
        if (filterId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such filter");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing filter");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::SetSyslogFilterActive};
    status = db->runQuery(
        cdsQuery.setDltFilterActive(
            Query::Type::SQLite3, slgFilter.name(), slgFilter.owner(), slgFilter.active()),
        query);
    if (!status) {
        mrq.args.emplace(Defaults::Arg::Reason, "Failed to set Syslog filter active state");
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Syslog filter active state set");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doGetSyslogFilters(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto userId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB GetSyslogFilters request from client: " << rq.client->getName();
    const auto &userData = std::any_cast<cds::msg::server::UserData>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::GetUserId};
    queryCheckExisting.raw = &userId;
    auto status = db->runQuery(cdsQuery.getUserId(Query::Type::SQLite3, userData.name()),
                               queryCheckExisting);
    if (status) {
        if (userId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such user");
            return CDSApp()->getDispatcher()->pushRequest(mrq);
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Status, cdsDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing filter");
        return CDSApp()->getDispatcher()->pushRequest(mrq);
    }

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::GetSyslogFilters};
    auto querySyslogFilters = std::vector<cds::msg::server::SyslogFilter>();
    query.raw = &querySyslogFilters;

    status = db->runQuery(cdsQuery.getSyslogFilters(Query::Type::SQLite3, userData.name()), query);
    if (!status) {
        mrq.args.emplace(Defaults::Arg::Reason, "Failed to get Syslog filters");
    } else {
        cds::msg::Envelope envelope;
        cds::msg::server::Message message;
        cds::msg::server::SyslogFilters slgFilters;

        for (auto &slgFilter : querySyslogFilters) {
            auto tmpFilter = slgFilters.add_filter();
            tmpFilter->CopyFrom(slgFilter);
        }

        message.set_type(msg::server::Message_Type_SyslogFilters);
        message.mutable_data()->PackFrom(slgFilters);
        envelope.mutable_mesg()->PackFrom(message);

        envelope.set_target(msg::Envelope_Recipient_Viewer);
        envelope.set_origin(msg::Envelope_Recipient_Server);

        status = rq.client->writeEnvelope(envelope);
        if (!status) {
            logWarn() << "Fail to send Syslog filters to client " << rq.client->getFD();
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to send Syslog filters");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "Syslog filters provided");
        }
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? cdsDefaults.valFor(Defaults::Val::StatusOkay)
                                    : cdsDefaults.valFor(Defaults::Val::StatusError));
    return CDSApp()->getDispatcher()->pushRequest(mrq);
}

static auto doConnect(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    return true;
}

static auto doDisconnect(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    return true;
}

static auto doGetEntries(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    logDebug() << "Handling DB GetEntries request from client: " << rq.client->getName();
    return true;
}

} // namespace cds::server

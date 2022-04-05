#include "SQLiteDatabase.h"
#include "Application.h"
#include "Defaults.h"
#include "Query.h"

#include "Collector.pb.h"
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

namespace tkm::collector
{

static auto sqlite_callback(void *data, int argc, char **argv, char **colname) -> int;
static auto doCheckDatabase(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doInitDatabase(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doLoadDevices(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doGetDevices(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doGetSessions(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doAddDevice(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool;
static auto doRemoveDevice(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doConnect(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool;
static auto doDisconnect(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doStartDeviceSession(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doStopDeviceSession(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doGetEntries(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doAddSession(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doEndSession(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doCleanSessions(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool;
static auto doAddData(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool;

SQLiteDatabase::SQLiteDatabase()
: IDatabase()
{
    fs::path addr(CollectorApp()->getOptions()->getFor(Options::Key::DBServerAddress));
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
    CollectorApp()->addEventSource(m_queue);

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
    case SQLiteDatabase::QueryType::AddDevice:
    case SQLiteDatabase::QueryType::RemDevice:
    case SQLiteDatabase::QueryType::AddSession:
    case SQLiteDatabase::QueryType::EndSession:
    case SQLiteDatabase::QueryType::AddData:
        break;
    case SQLiteDatabase::QueryType::LoadDevices:
    case SQLiteDatabase::QueryType::GetDevices: {
        auto pld = static_cast<std::vector<tkm::msg::collector::DeviceData> *>(query->raw);
        tkm::msg::collector::DeviceData device {};
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i], tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Id).c_str(), 60)
                == 0) {
                device.set_id(std::stol(argv[i]));
            } else if (strncmp(colname[i],
                               tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Hash).c_str(),
                               60)
                       == 0) {
                device.set_hash(argv[i]);
            } else if (strncmp(colname[i],
                               tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Name).c_str(),
                               60)
                       == 0) {
                device.set_name(argv[i]);
            } else if (strncmp(colname[i],
                               tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Address).c_str(),
                               60)
                       == 0) {
                device.set_address(argv[i]);
            } else if (strncmp(colname[i],
                               tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Port).c_str(),
                               60)
                       == 0) {
                device.set_port(std::stoi(argv[i]));
            } else if (strncmp(colname[i],
                               tkmQuery.m_deviceColumn.at(Query::DeviceColumn::State).c_str(),
                               60)
                       == 0) {
                device.set_state(
                    static_cast<tkm::msg::collector::DeviceData_State>(std::stoi(argv[i])));
            }
        }
        pld->emplace_back(device);
        break;
    }
    case SQLiteDatabase::QueryType::HasDevice: {
        auto pld = static_cast<int *>(query->raw);
        for (int i = 0; i < argc; i++) {
            if (strncmp(colname[i], tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Id).c_str(), 60)
                == 0) {
                *pld = std::stol(argv[i]);
            }
        }
        break;
    }
    case SQLiteDatabase::QueryType::CleanSessions:
    case SQLiteDatabase::QueryType::GetSessions: {
        auto pld = static_cast<std::vector<tkm::msg::collector::SessionData> *>(query->raw);
        tkm::msg::collector::SessionData session {};
        for (int i = 0; i < argc; i++) {
            if (strncmp(
                    colname[i], tkmQuery.m_sessionColumn.at(Query::SessionColumn::Id).c_str(), 60)
                == 0) {
                session.set_id(std::stol(argv[i]));
            } else if (strncmp(colname[i],
                               tkmQuery.m_sessionColumn.at(Query::SessionColumn::Hash).c_str(),
                               60)
                       == 0) {
                session.set_hash(argv[i]);
            } else if (strncmp(colname[i],
                               tkmQuery.m_sessionColumn.at(Query::SessionColumn::Name).c_str(),
                               60)
                       == 0) {
                session.set_name(argv[i]);
            } else if (strncmp(colname[i],
                               tkmQuery.m_sessionColumn.at(Query::SessionColumn::StartTimestamp)
                                   .c_str(),
                               60)
                       == 0) {
                session.set_started(std::stol(argv[i]));
            } else if (strncmp(
                           colname[i],
                           tkmQuery.m_sessionColumn.at(Query::SessionColumn::EndTimestamp).c_str(),
                           60)
                       == 0) {
                session.set_ended(std::stol(argv[i]));
            }
        }
        pld->emplace_back(session);
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
    case IDatabase::Action::Connect:
        return doConnect(getShared(), rq);
    case IDatabase::Action::Disconnect:
        return doDisconnect(getShared(), rq);
    case IDatabase::Action::LoadDevices:
        return doLoadDevices(getShared(), rq);
    case IDatabase::Action::GetDevices:
        return doGetDevices(getShared(), rq);
    case IDatabase::Action::AddDevice:
        return doAddDevice(getShared(), rq);
    case IDatabase::Action::RemoveDevice:
        return doRemoveDevice(getShared(), rq);
    case IDatabase::Action::GetSessions:
        return doGetSessions(getShared(), rq);
    case IDatabase::Action::AddSession:
        return doAddSession(getShared(), rq);
    case IDatabase::Action::EndSession:
        return doEndSession(getShared(), rq);
    case IDatabase::Action::CleanSessions:
        return doCleanSessions(getShared(), rq);
    default:
        break;
    }
    logError() << "Unknown action request";
    return false;
}

static auto doCheckDatabase(const shared_ptr<SQLiteDatabase> &db, const SQLiteDatabase::Request &rq)
    -> bool
{
    // TODO: Handle database check
    return true;
}

static auto doInitDatabase(const shared_ptr<SQLiteDatabase> &db, const SQLiteDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    logDebug() << "Handling DB init request";

    if (rq.args.count(Defaults::Arg::Forced)) {
        if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
            SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::DropTables};
            db->runQuery(tkmQuery.dropTables(Query::Type::SQLite3), query);
        }
    }

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::Create};
    auto status = db->runQuery(tkmQuery.createTables(Query::Type::SQLite3), query);

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }
    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                    : tkmDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason,
                     status == true ? "Database init complete"
                                    : "Database init failed. Query error");

    return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doLoadDevices(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    logDebug() << "Handling DB LoadDevices";

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::LoadDevices};
    auto queryDeviceList = std::vector<tkm::msg::collector::DeviceData>();
    query.raw = &queryDeviceList;

    auto status = db->runQuery(tkmQuery.getDevices(Query::Type::SQLite3), query);
    if (status) {
        for (auto &deviceData : queryDeviceList) {
            if (CollectorApp()->getDeviceManager()->getDevice(deviceData.hash()) != nullptr)
                continue;

            auto newDevice = std::make_shared<MonitorDevice>(deviceData);
            CollectorApp()->getDeviceManager()->addDevice(newDevice);
            newDevice->getDeviceData().set_state(tkm::msg::collector::DeviceData_State_Loaded);
            newDevice->enableEvents();
        }
    } else {
        logError() << "Failed to load devices";
    }

    return true;
}

static auto doCleanSessions(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    logDebug() << "Handling DB CleanSessions";

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::CleanSessions};
    auto querySessionList = std::vector<tkm::msg::collector::SessionData>();
    query.raw = &querySessionList;

    auto status = db->runQuery(tkmQuery.getSessions(Query::Type::SQLite3), query);
    if (status) {
        for (auto &sessionData : querySessionList) {
            if (sessionData.ended() == 0) {
                IDatabase::Request dbrq {.action = IDatabase::Action::EndSession};
                dbrq.args.emplace(Defaults::Arg::SessionHash, sessionData.hash());
                db->pushRequest(dbrq);
            }
        }
    } else {
        logError() << "Failed to clean sessions";
    }

    return true;
}

static auto doGetDevices(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB GetDevices request from client: " << rq.client->getName();

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::GetDevices};
    auto queryDevList = std::vector<tkm::msg::collector::DeviceData>();
    query.raw = &queryDevList;

    auto status = db->runQuery(tkmQuery.getDevices(Query::Type::SQLite3), query);
    if (status) {
        tkm::msg::Envelope envelope;
        tkm::msg::collector::Message message;
        tkm::msg::collector::DeviceList devList;

        for (auto &dev : queryDevList) {
            auto activeDevice = CollectorApp()->getDeviceManager()->getDevice(dev.hash());

            if (activeDevice != nullptr) {
                dev.set_state(activeDevice->getDeviceData().state());
            }

            auto tmpDev = devList.add_device();
            tmpDev->CopyFrom(dev);
        }

        message.set_type(tkm::msg::collector::Message_Type_DeviceList);
        message.mutable_data()->PackFrom(devList);
        envelope.mutable_mesg()->PackFrom(message);

        envelope.set_target(msg::Envelope_Recipient_Any);
        envelope.set_origin(msg::Envelope_Recipient_Collector);

        if (!rq.client->writeEnvelope(envelope)) {
            logWarn() << "Fail to send device list to client " << rq.client->getFD();
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to send device list");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "List provided");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Query failed");
        logError() << "Query error for getUsers";
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                    : tkmDefaults.valFor(Defaults::Val::StatusError));

    return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doGetSessions(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB GetSessions request from client: " << rq.client->getName();
    const auto &deviceData = std::any_cast<tkm::msg::collector::DeviceData>(rq.bulkData);

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::GetSessions};
    auto querySessionList = std::vector<tkm::msg::collector::SessionData>();
    query.raw = &querySessionList;

    auto status
        = db->runQuery(tkmQuery.getSessions(Query::Type::SQLite3, deviceData.hash()), query);
    if (status) {
        tkm::msg::Envelope envelope;
        tkm::msg::collector::Message message;
        tkm::msg::collector::SessionList sessionList;

        for (auto &session : querySessionList) {
            if (session.ended() == 0) {
                session.set_state(tkm::msg::collector::SessionData_State_Progress);
            } else {
                session.set_state(tkm::msg::collector::SessionData_State_Complete);
            }
            auto tmpSession = sessionList.add_session();
            tmpSession->CopyFrom(session);
        }

        message.set_type(tkm::msg::collector::Message_Type_SessionList);
        message.mutable_data()->PackFrom(sessionList);
        envelope.mutable_mesg()->PackFrom(message);

        envelope.set_target(msg::Envelope_Recipient_Any);
        envelope.set_origin(msg::Envelope_Recipient_Collector);

        if (!rq.client->writeEnvelope(envelope)) {
            logWarn() << "Fail to send session list to client " << rq.client->getFD();
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to send session list");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "List provided");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Query failed");
        logError() << "Query error for getUsers";
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                    : tkmDefaults.valFor(Defaults::Val::StatusError));

    return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doAddDevice(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto devId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB AddDevice request from client: " << rq.client->getName();
    const auto &deviceData = std::any_cast<tkm::msg::collector::DeviceData>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::HasDevice};
    queryCheckExisting.raw = &devId;
    auto status = db->runQuery(tkmQuery.hasDevice(Query::Type::SQLite3, deviceData.hash()),
                               queryCheckExisting);
    if (status) {
        if (rq.args.count(Defaults::Arg::Forced)) {
            if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
                SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::RemDevice};
                db->runQuery(tkmQuery.remDevice(Query::Type::SQLite3, deviceData.hash()), query);
            }
        } else if (devId != -1) {
            mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "Device already exists");
            return CollectorApp()->getDispatcher()->pushRequest(mrq);
        }

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::AddDevice};
        status = db->runQuery(tkmQuery.addDevice(Query::Type::SQLite3,
                                                 deviceData.hash(),
                                                 deviceData.name(),
                                                 deviceData.address(),
                                                 deviceData.port()),
                              query);
        if (!status) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to add device");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "Device added");
            CollectorApp()->getDeviceManager()->loadDevices();
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing device");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                    : tkmDefaults.valFor(Defaults::Val::StatusError));
    return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doRemoveDevice(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    auto devId = -1;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    logDebug() << "Handling DB RemoveDevice request from client: " << rq.client->getName();
    const auto &deviceData = std::any_cast<tkm::msg::collector::DeviceData>(rq.bulkData);

    SQLiteDatabase::Query queryCheckExisting {.type = SQLiteDatabase::QueryType::HasDevice};
    queryCheckExisting.raw = &devId;

    auto status = db->runQuery(tkmQuery.hasDevice(Query::Type::SQLite3, deviceData.hash()),
                               queryCheckExisting);
    if (status) {
        if (devId == -1) {
            mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
            mrq.args.emplace(Defaults::Arg::Reason, "No such device");
        }

        SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::RemDevice};
        status = db->runQuery(tkmQuery.remDevice(Query::Type::SQLite3, deviceData.hash()), query);

        if (!status) {
            mrq.args.emplace(Defaults::Arg::Reason, "Failed to remove device");
        } else {
            mrq.args.emplace(Defaults::Arg::Reason, "Device removed");
        }
    } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
    }

    mrq.args.emplace(Defaults::Arg::Status,
                     status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                    : tkmDefaults.valFor(Defaults::Val::StatusError));
    return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doAddSession(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    logDebug() << "Handling DB AddSession request";
    if ((rq.args.count(Defaults::Arg::DeviceHash) == 0)
        || (rq.args.count(Defaults::Arg::SessionHash) == 0)) {
        logError() << "Invalid session data";
        return true;
    }

    const std::string sessionName
        = "Collector." + std::to_string(getpid()) + "." + std::to_string(time(NULL));

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::AddSession};
    auto status = db->runQuery(tkmQuery.addSession(Query::Type::SQLite3,
                                                   rq.args.at(Defaults::Arg::SessionHash),
                                                   sessionName,
                                                   time(NULL),
                                                   rq.args.at(Defaults::Arg::DeviceHash)),
                               query);
    if (!status) {
        logError() << "Query failed to add session";
    }

    return true;
}

static auto doEndSession(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    logDebug() << "Handling DB EndSession request";
    if ((rq.args.count(Defaults::Arg::SessionHash) == 0)) {
        logError() << "Invalid session data";
        return true;
    }

    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::EndSession};
    auto status = db->runQuery(
        tkmQuery.endSession(Query::Type::SQLite3, rq.args.at(Defaults::Arg::SessionHash)), query);
    if (!status) {
        logError() << "Query failed to mark end session";
    }

    return true;
}

static auto doAddData(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    SQLiteDatabase::Query query {.type = SQLiteDatabase::QueryType::AddData};
    const auto &data = std::any_cast<tkm::msg::server::Data>(rq.bulkData);
    bool status = true;

    if ((rq.args.count(Defaults::Arg::SessionHash) == 0)) {
        logError() << "Invalid session data";
        return true;
    }

    auto writeProcAcct = [&db, &rq, &status, &query](const std::string &sessionHash,
                                                     const tkm::msg::server::ProcAcct &acct,
                                                     uint64_t ts) {
        status = db->runQuery(tkmQuery.addData(Query::Type::SQLite3, sessionHash, acct, ts), query);
    };

    auto writeSysProcStat
        = [&db, &rq, &status, &query](const std::string &sessionHash,
                                      const tkm::msg::server::SysProcStat &sysProcStat,
                                      uint64_t ts) {
              status = db->runQuery(
                  tkmQuery.addData(Query::Type::SQLite3, sessionHash, sysProcStat, ts), query);
          };

    auto writeSysProcPressure
        = [&db, &rq, &status, &query](const std::string &sessionHash,
                                      const tkm::msg::server::SysProcPressure &sysProcPressure,
                                      uint64_t ts) {
              status = db->runQuery(
                  tkmQuery.addData(Query::Type::SQLite3, sessionHash, sysProcPressure, ts), query);
          };

    switch (data.what()) {
    case tkm::msg::server::Data_What_ProcAcct: {
        tkm::msg::server::ProcAcct procAcct;
        data.payload().UnpackTo(&procAcct);
        writeProcAcct(rq.args.at(Defaults::Arg::SessionHash), procAcct, data.timestamp());
        break;
    }
    case tkm::msg::server::Data_What_SysProcStat: {
        tkm::msg::server::SysProcStat sysProcStat;
        data.payload().UnpackTo(&sysProcStat);
        writeSysProcStat(rq.args.at(Defaults::Arg::SessionHash), sysProcStat, data.timestamp());
        break;
    }
    case tkm::msg::server::Data_What_SysProcPressure: {
        tkm::msg::server::SysProcPressure sysProcPressure;
        data.payload().UnpackTo(&sysProcPressure);
        writeSysProcPressure(
            rq.args.at(Defaults::Arg::SessionHash), sysProcPressure, data.timestamp());
        break;
    }
    default:
        break;
    }

    return true;
}

static auto doConnect(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    // No need for DB connect with SQLite
    return true;
}

static auto doDisconnect(const shared_ptr<SQLiteDatabase> &db, const IDatabase::Request &rq) -> bool
{
    // No need for DB disconnect with SQLite
    return true;
}

} // namespace tkm::collector

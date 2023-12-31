/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SQLiteDatabase Class
 * @details   SQLite3 database implementation
 *-
 */

#include "SQLiteDatabase.h"
#include "Application.h"
#include "Defaults.h"
#include "Query.h"

#include <Helpers.h>
#include <any>
#include <filesystem>
#include <string>
#include <taskmonitor/taskmonitor.h>
#include <vector>

namespace tkm::collector
{

static auto sqlite_callback(void *data, int argc, char **argv, char **colname) -> int;
static bool doCheckDatabase(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doInitDatabase(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doLoadDevices(const std::shared_ptr<SQLiteDatabase> db);
static bool doGetDevices(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doGetSessions(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doAddDevice(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doRemoveDevice(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doConnect(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doDisconnect(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doStartDeviceSession(const std::shared_ptr<SQLiteDatabase> db,
                                 const IDatabase::Request &rq);
static bool doStopDeviceSession(const std::shared_ptr<SQLiteDatabase> db,
                                const IDatabase::Request &rq);
static bool doGetEntries(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doAddSession(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doRemSession(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doEndSession(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);
static bool doCleanSessions(const std::shared_ptr<SQLiteDatabase> db);
static bool doAddData(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq);

SQLiteDatabase::SQLiteDatabase(std::shared_ptr<Options> options)
: IDatabase(options)
{
  std::filesystem::path addr(CollectorApp()->getOptions()->getFor(Options::Key::DBFilePath));
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

  IDatabase::Request dbrq{.client = nullptr,
                          .action = IDatabase::Action::CheckDatabase,
                          .args = std::map<Defaults::Arg, std::string>(),
                          .bulkData = std::make_any<int>(0)};
  pushRequest(dbrq);
}

bool SQLiteDatabase::runQuery(const std::string &sql, SQLiteDatabase::Query &query)
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
  case SQLiteDatabase::QueryType::RemSession:
  case SQLiteDatabase::QueryType::AddData:
    break;
  case SQLiteDatabase::QueryType::LoadDevices:
  case SQLiteDatabase::QueryType::GetDevices: {
    auto pld = static_cast<std::vector<tkm::msg::control::DeviceData> *>(query->raw);
    tkm::msg::control::DeviceData device{};
    for (int i = 0; i < argc; i++) {
      if (strncmp(colname[i], tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Id).c_str(), 60) ==
          0) {
        device.set_id(std::stol(argv[i]));
      } else if (strncmp(colname[i],
                         tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Hash).c_str(),
                         60) == 0) {
        device.set_hash(argv[i]);
      } else if (strncmp(colname[i],
                         tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Name).c_str(),
                         60) == 0) {
        device.set_name(argv[i]);
      } else if (strncmp(colname[i],
                         tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Address).c_str(),
                         60) == 0) {
        device.set_address(argv[i]);
      } else if (strncmp(colname[i],
                         tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Port).c_str(),
                         60) == 0) {
        device.set_port(std::stoi(argv[i]));
      }
    }
    pld->emplace_back(device);
    break;
  }
  case SQLiteDatabase::QueryType::HasDevice: {
    auto pld = static_cast<int *>(query->raw);
    for (int i = 0; i < argc; i++) {
      if (strncmp(colname[i], tkmQuery.m_deviceColumn.at(Query::DeviceColumn::Id).c_str(), 60) ==
          0) {
        *pld = static_cast<int>(std::stol(argv[i]));
      }
    }
    break;
  }
  case SQLiteDatabase::QueryType::HasSession: {
    auto pld = static_cast<int *>(query->raw);
    for (int i = 0; i < argc; i++) {
      if (strncmp(colname[i], tkmQuery.m_sessionColumn.at(Query::SessionColumn::Id).c_str(), 60) ==
          0) {
        *pld = static_cast<int>(std::stol(argv[i]));
      }
    }
    break;
  }
  case SQLiteDatabase::QueryType::CleanSessions:
  case SQLiteDatabase::QueryType::GetSessions: {
    auto pld = static_cast<std::vector<tkm::msg::control::SessionData> *>(query->raw);
    tkm::msg::control::SessionData session{};
    for (int i = 0; i < argc; i++) {
      if (strncmp(colname[i], tkmQuery.m_sessionColumn.at(Query::SessionColumn::Id).c_str(), 60) ==
          0) {
        session.set_id(std::stol(argv[i]));
      } else if (strncmp(colname[i],
                         tkmQuery.m_sessionColumn.at(Query::SessionColumn::Hash).c_str(),
                         60) == 0) {
        session.set_hash(argv[i]);
      } else if (strncmp(colname[i],
                         tkmQuery.m_sessionColumn.at(Query::SessionColumn::Name).c_str(),
                         60) == 0) {
        session.set_name(argv[i]);
      } else if (strncmp(colname[i],
                         tkmQuery.m_sessionColumn.at(Query::SessionColumn::StartTimestamp).c_str(),
                         60) == 0) {
        session.set_started(std::stoul(argv[i]));
      } else if (strncmp(colname[i],
                         tkmQuery.m_sessionColumn.at(Query::SessionColumn::EndTimestamp).c_str(),
                         60) == 0) {
        session.set_ended(std::stoul(argv[i]));
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

bool SQLiteDatabase::requestHandler(const Request &rq)
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
    return doLoadDevices(getShared());
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
  case IDatabase::Action::RemSession:
    return doRemSession(getShared(), rq);
  case IDatabase::Action::EndSession:
    return doEndSession(getShared(), rq);
  case IDatabase::Action::CleanSessions:
    return doCleanSessions(getShared());
  case IDatabase::Action::AddData:
    return doAddData(getShared(), rq);
  default:
    break;
  }
  logError() << "Unknown action request";
  return false;
}

static bool doCheckDatabase(const std::shared_ptr<SQLiteDatabase> db,
                            const SQLiteDatabase::Request &rq)
{
  // TODO: Handle database check
  static_cast<void>(db); // UNUSED
  static_cast<void>(rq); // UNUSED
  return true;
}

static bool doInitDatabase(const std::shared_ptr<SQLiteDatabase> db,
                           const SQLiteDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client,
                          .action = Dispatcher::Action::SendStatus,
                          .args = std::map<Defaults::Arg, std::string>(),
                          .bulkData = std::make_any<int>(0)};

  logDebug() << "Handling DB init request";

  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::DropTables, .raw = nullptr};
      db->runQuery(tkmQuery.dropTables(Query::Type::SQLite3), query);
    }
  }

  SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::Create, .raw = nullptr};
  auto status = db->runQuery(tkmQuery.createTables(Query::Type::SQLite3), query);

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }
  mrq.args.emplace(Defaults::Arg::Status,
                   status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                  : tkmDefaults.valFor(Defaults::Val::StatusError));
  mrq.args.emplace(Defaults::Arg::Reason,
                   status == true ? "Database init complete" : "Database init failed. Query error");

  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static bool doLoadDevices(const std::shared_ptr<SQLiteDatabase> db)
{
  logDebug() << "Handling DB LoadDevices";

  auto queryDeviceList = std::vector<tkm::msg::control::DeviceData>();
  SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::LoadDevices,
                              .raw = &queryDeviceList};

  auto status = db->runQuery(tkmQuery.getDevices(Query::Type::SQLite3), query);
  if (status) {
    for (auto &deviceData : queryDeviceList) {
      if (CollectorApp()->getDeviceManager()->getDevice(deviceData.hash()) != nullptr)
        continue;

      auto newDevice = std::make_shared<MonitorDevice>(deviceData);
      CollectorApp()->getDeviceManager()->addDevice(newDevice);
      newDevice->getDeviceData().set_state(tkm::msg::control::DeviceData_State_Loaded);
      newDevice->enableEvents();
    }
  } else {
    logError() << "Failed to load devices";
  }

  return true;
}

static bool doCleanSessions(const std::shared_ptr<SQLiteDatabase> db)
{
  logDebug() << "Handling DB CleanSessions";

  auto querySessionList = std::vector<tkm::msg::control::SessionData>();
  SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::CleanSessions,
                              .raw = &querySessionList};

  auto status = db->runQuery(tkmQuery.getSessions(Query::Type::SQLite3), query);
  if (status) {
    for (auto &sessionData : querySessionList) {
      if (sessionData.ended() == 0) {
        IDatabase::Request dbrq{.client = nullptr,
                                .action = IDatabase::Action::EndSession,
                                .args = std::map<Defaults::Arg, std::string>(),
                                .bulkData = std::make_any<int>(0)};
        dbrq.args.emplace(Defaults::Arg::SessionHash, sessionData.hash());
        db->pushRequest(dbrq);
      }
    }
  } else {
    logError() << "Failed to clean sessions";
  }

  return true;
}

static bool doGetDevices(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client,
                          .action = Dispatcher::Action::SendStatus,
                          .args = std::map<Defaults::Arg, std::string>(),
                          .bulkData = std::make_any<int>(0)};

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB GetDevices request from client: " << rq.client->getName();

  auto queryDevList = std::vector<tkm::msg::control::DeviceData>();
  SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::GetDevices, .raw = &queryDevList};

  auto status = db->runQuery(tkmQuery.getDevices(Query::Type::SQLite3), query);
  if (status) {
    tkm::msg::Envelope envelope;
    tkm::msg::control::Message message;
    tkm::msg::control::DeviceList devList;

    for (auto &dev : queryDevList) {
      auto activeDevice = CollectorApp()->getDeviceManager()->getDevice(dev.hash());

      if (activeDevice != nullptr) {
        dev.set_state(activeDevice->getDeviceData().state());
      }

      auto tmpDev = devList.add_device();
      tmpDev->CopyFrom(dev);
    }

    message.set_type(tkm::msg::control::Message_Type_DeviceList);
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

static bool doGetSessions(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client,
                          .action = Dispatcher::Action::SendStatus,
                          .args = std::map<Defaults::Arg, std::string>(),
                          .bulkData = std::make_any<int>(0)};
  bool status = false;

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB GetSessions request from client: " << rq.client->getName();
  const auto &deviceData = std::any_cast<tkm::msg::control::DeviceData>(rq.bulkData);

  auto querySessionList = std::vector<tkm::msg::control::SessionData>();
  SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::GetSessions,
                              .raw = &querySessionList};

  if (deviceData.hash().empty()) {
    status = db->runQuery(tkmQuery.getSessions(Query::Type::SQLite3), query);
  } else {
    status = db->runQuery(tkmQuery.getSessions(Query::Type::SQLite3, deviceData.hash()), query);
  }

  if (status) {
    tkm::msg::Envelope envelope;
    tkm::msg::control::Message message;
    tkm::msg::control::SessionList sessionList;

    for (auto &session : querySessionList) {
      if (session.ended() == 0) {
        session.set_state(tkm::msg::control::SessionData_State_Progress);
      } else {
        session.set_state(tkm::msg::control::SessionData_State_Complete);
      }
      auto tmpSession = sessionList.add_session();
      tmpSession->CopyFrom(session);
    }

    message.set_type(tkm::msg::control::Message_Type_SessionList);
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

static bool doAddDevice(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client,
                          .action = Dispatcher::Action::SendStatus,
                          .args = std::map<Defaults::Arg, std::string>(),
                          .bulkData = std::make_any<int>(0)};

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB AddDevice request from client: " << rq.client->getName();
  const auto &deviceData = std::any_cast<tkm::msg::control::DeviceData>(rq.bulkData);

  auto devId = -1;
  SQLiteDatabase::Query queryCheckExisting{.type = SQLiteDatabase::QueryType::HasDevice,
                                           .raw = &devId};
  auto status =
      db->runQuery(tkmQuery.hasDevice(Query::Type::SQLite3, deviceData.hash()), queryCheckExisting);
  if (status) {
    if (rq.args.count(Defaults::Arg::Forced)) {
      if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
        SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::RemDevice, .raw = nullptr};
        db->runQuery(tkmQuery.remDevice(Query::Type::SQLite3, deviceData.hash()), query);
      }
    } else if (devId != -1) {
      mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
      mrq.args.emplace(Defaults::Arg::Reason, "Device already exists");
      return CollectorApp()->getDispatcher()->pushRequest(mrq);
    }

    SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::AddDevice, .raw = nullptr};
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

static bool doRemoveDevice(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client,
                          .action = Dispatcher::Action::SendStatus,
                          .args = std::map<Defaults::Arg, std::string>(),
                          .bulkData = std::make_any<int>(0)};

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB RemoveDevice request from client: " << rq.client->getName();
  const auto &deviceData = std::any_cast<tkm::msg::control::DeviceData>(rq.bulkData);

  auto devId = -1;
  SQLiteDatabase::Query queryCheckExisting{.type = SQLiteDatabase::QueryType::HasDevice,
                                           .raw = &devId};

  auto status =
      db->runQuery(tkmQuery.hasDevice(Query::Type::SQLite3, deviceData.hash()), queryCheckExisting);
  if (status) {
    if (devId == -1) {
      mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
      mrq.args.emplace(Defaults::Arg::Reason, "No such device");
    }

    SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::RemDevice, .raw = nullptr};
    status = db->runQuery(tkmQuery.remDevice(Query::Type::SQLite3, deviceData.hash()), query);

    if (!status) {
      mrq.args.emplace(Defaults::Arg::Reason, "Failed to remove device");
    } else {
      mrq.args.emplace(Defaults::Arg::Reason, "Device removed");
    }
  } else {
    mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing device");
  }

  mrq.args.emplace(Defaults::Arg::Status,
                   status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                  : tkmDefaults.valFor(Defaults::Val::StatusError));
  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static bool doAddSession(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq)
{
  const auto &sessionInfo = std::any_cast<tkm::msg::monitor::SessionInfo>(rq.bulkData);

  logDebug() << "Handling DB AddSession request";
  if (rq.args.count(Defaults::Arg::DeviceHash) == 0) {
    logError() << "Invalid session data";
    throw std::runtime_error("Invalid arguments");
  }

  // We don't allow sessions with the same hash, so if a device is sending an existing session hash
  // we disconnect the device. Maybe is to harsh but this should not happen often in practice
  auto sesId = -1;
  SQLiteDatabase::Query queryCheckExisting{.type = SQLiteDatabase::QueryType::HasSession,
                                           .raw = &sesId};

  auto status = db->runQuery(tkmQuery.hasSession(Query::Type::SQLite3, sessionInfo.hash()),
                             queryCheckExisting);
  if (status) {
    if (sesId != -1) {
      logError() << "Session hash collision detected. Remove old session " << sessionInfo.hash();
      SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::RemSession, .raw = nullptr};
      status = db->runQuery(tkmQuery.remSession(Query::Type::SQLite3, sessionInfo.hash()), query);
      if (!status) {
        logError() << "Failed to remove existing session";
      }
    }
  } else {
    logError() << "Failed to check existing session";
  }

  SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::AddSession, .raw = nullptr};
  status = db->runQuery(tkmQuery.addSession(Query::Type::SQLite3,
                                            sessionInfo,
                                            rq.args.at(Defaults::Arg::DeviceHash),
                                            static_cast<uint64_t>(time(NULL))),
                        query);
  if (!status) {
    logError() << "Query failed to add session";
  }

  return true;
}

static bool doRemSession(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client,
                          .action = Dispatcher::Action::SendStatus,
                          .args = std::map<Defaults::Arg, std::string>(),
                          .bulkData = std::make_any<int>(0)};

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB RemoveDevice request from client: " << rq.client->getName();
  const auto &sessionData = std::any_cast<tkm::msg::control::SessionData>(rq.bulkData);

  auto sesId = -1;
  SQLiteDatabase::Query queryCheckExisting{.type = SQLiteDatabase::QueryType::HasSession,
                                           .raw = &sesId};

  auto status = db->runQuery(tkmQuery.hasSession(Query::Type::SQLite3, sessionData.hash()),
                             queryCheckExisting);
  if (status) {
    if (sesId == -1) {
      mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
      mrq.args.emplace(Defaults::Arg::Reason, "No such session");
    }

    SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::RemSession, .raw = nullptr};
    status = db->runQuery(tkmQuery.remSession(Query::Type::SQLite3, sessionData.hash()), query);

    if (!status) {
      mrq.args.emplace(Defaults::Arg::Reason, "Failed to remove session");
    } else {
      mrq.args.emplace(Defaults::Arg::Reason, "Session removed");
    }
  } else {
    mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing session");
  }

  mrq.args.emplace(Defaults::Arg::Status,
                   status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                  : tkmDefaults.valFor(Defaults::Val::StatusError));
  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static bool doEndSession(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq)
{
  logDebug() << "Handling DB EndSession request";
  if ((rq.args.count(Defaults::Arg::SessionHash) == 0)) {
    logError() << "Invalid session data";
    throw std::runtime_error("Invalid arguments");
  }

  SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::EndSession, .raw = nullptr};
  auto status = db->runQuery(
      tkmQuery.endSession(Query::Type::SQLite3, rq.args.at(Defaults::Arg::SessionHash)), query);
  if (!status) {
    logError() << "Query failed to mark end session";
  }

  return true;
}

static bool doAddData(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq)
{
  SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::AddData, .raw = nullptr};
  const auto &data = std::any_cast<tkm::msg::monitor::Data>(rq.bulkData);
  bool status = true;

  if ((rq.args.count(Defaults::Arg::SessionHash) == 0)) {
    logError() << "Invalid session data";
    throw std::runtime_error("Invalid arguments");
  }

  auto writeProcAcct = [&db, &status, &query](const std::string &sessionHash,
                                              const tkm::msg::monitor::ProcAcct &acct,
                                              uint64_t systemTime,
                                              uint64_t monotonicTime,
                                              uint64_t receiveTime) {
    status = db->runQuery(
        tkmQuery.addData(
            Query::Type::SQLite3, sessionHash, acct, systemTime, monotonicTime, receiveTime),
        query);
  };

  auto writeProcInfo = [&db, &status, &query](const std::string &sessionHash,
                                              const tkm::msg::monitor::ProcInfo &info,
                                              uint64_t systemTime,
                                              uint64_t monotonicTime,
                                              uint64_t receiveTime) {
    status = db->runQuery(
        tkmQuery.addData(
            Query::Type::SQLite3, sessionHash, info, systemTime, monotonicTime, receiveTime),
        query);
  };

  auto writeContextInfo = [&db, &status, &query](const std::string &sessionHash,
                                                 const tkm::msg::monitor::ContextInfo &info,
                                                 uint64_t systemTime,
                                                 uint64_t monotonicTime,
                                                 uint64_t receiveTime) {
    status = db->runQuery(
        tkmQuery.addData(
            Query::Type::SQLite3, sessionHash, info, systemTime, monotonicTime, receiveTime),
        query);
  };

  auto writeSysProcStat = [&db, &status, &query](const std::string &sessionHash,
                                                 const tkm::msg::monitor::SysProcStat &sysProcStat,
                                                 uint64_t systemTime,
                                                 uint64_t monotonicTime,
                                                 uint64_t receiveTime) {
    status = db->runQuery(
        tkmQuery.addData(
            Query::Type::SQLite3, sessionHash, sysProcStat, systemTime, monotonicTime, receiveTime),
        query);
  };

  auto writeSysProcBuddyInfo =
      [&db, &status, &query](const std::string &sessionHash,
                             const tkm::msg::monitor::SysProcBuddyInfo &sysProcBuddyInfo,
                             uint64_t systemTime,
                             uint64_t monotonicTime,
                             uint64_t receiveTime) {
        status = db->runQuery(tkmQuery.addData(Query::Type::SQLite3,
                                               sessionHash,
                                               sysProcBuddyInfo,
                                               systemTime,
                                               monotonicTime,
                                               receiveTime),
                              query);
      };

  auto writeSysProcWireless =
      [&db, &status, &query](const std::string &sessionHash,
                             const tkm::msg::monitor::SysProcWireless &sysProcWireless,
                             uint64_t systemTime,
                             uint64_t monotonicTime,
                             uint64_t receiveTime) {
        status = db->runQuery(tkmQuery.addData(Query::Type::SQLite3,
                                               sessionHash,
                                               sysProcWireless,
                                               systemTime,
                                               monotonicTime,
                                               receiveTime),
                              query);
      };

  auto writeSysProcMemInfo = [&db, &status, &query](
                                 const std::string &sessionHash,
                                 const tkm::msg::monitor::SysProcMemInfo &sysProcMem,
                                 uint64_t systemTime,
                                 uint64_t monotonicTime,
                                 uint64_t receiveTime) {
    status = db->runQuery(
        tkmQuery.addData(
            Query::Type::SQLite3, sessionHash, sysProcMem, systemTime, monotonicTime, receiveTime),
        query);
  };

  auto writeSysProcPressure =
      [&db, &status, &query](const std::string &sessionHash,
                             const tkm::msg::monitor::SysProcPressure &sysProcPressure,
                             uint64_t systemTime,
                             uint64_t monotonicTime,
                             uint64_t receiveTime) {
        status = db->runQuery(tkmQuery.addData(Query::Type::SQLite3,
                                               sessionHash,
                                               sysProcPressure,
                                               systemTime,
                                               monotonicTime,
                                               receiveTime),
                              query);
      };

  auto writeSysProcDiskStats =
      [&db, &status, &query](const std::string &sessionHash,
                             const tkm::msg::monitor::SysProcDiskStats &sysProcDiskStats,
                             uint64_t systemTime,
                             uint64_t monotonicTime,
                             uint64_t receiveTime) {
        status = db->runQuery(tkmQuery.addData(Query::Type::SQLite3,
                                               sessionHash,
                                               sysProcDiskStats,
                                               systemTime,
                                               monotonicTime,
                                               receiveTime),
                              query);
      };

  auto writeProcEvent = [&db, &status, &query](const std::string &sessionHash,
                                               const tkm::msg::monitor::ProcEvent &procEvent,
                                               uint64_t systemTime,
                                               uint64_t monotonicTime,
                                               uint64_t receiveTime) {
    status = db->runQuery(
        tkmQuery.addData(
            Query::Type::SQLite3, sessionHash, procEvent, systemTime, monotonicTime, receiveTime),
        query);
  };

  auto writeSysProcVMStat = [&db, &status, &query](
                                 const std::string &sessionHash,
                                 const tkm::msg::monitor::SysProcVMStat &sysProcVMStat,
                                 uint64_t systemTime,
                                 uint64_t monotonicTime,
                                 uint64_t receiveTime) {
    status = db->runQuery(
        tkmQuery.addData(
            Query::Type::SQLite3, sessionHash, sysProcVMStat, systemTime, monotonicTime, receiveTime),
        query);
  };

  switch (data.what()) {
  case tkm::msg::monitor::Data_What_ProcEvent: {
    tkm::msg::monitor::ProcEvent procEvent;
    data.payload().UnpackTo(&procEvent);
    writeProcEvent(rq.args.at(Defaults::Arg::SessionHash),
                   procEvent,
                   data.system_time_sec(),
                   data.monotonic_time_sec(),
                   data.receive_time_sec());
    break;
  }
  case tkm::msg::monitor::Data_What_ProcAcct: {
    tkm::msg::monitor::ProcAcct procAcct;
    data.payload().UnpackTo(&procAcct);
    writeProcAcct(rq.args.at(Defaults::Arg::SessionHash),
                  procAcct,
                  data.system_time_sec(),
                  data.monotonic_time_sec(),
                  data.receive_time_sec());
    break;
  }
  case tkm::msg::monitor::Data_What_ProcInfo: {
    tkm::msg::monitor::ProcInfo procInfo;
    data.payload().UnpackTo(&procInfo);
    writeProcInfo(rq.args.at(Defaults::Arg::SessionHash),
                  procInfo,
                  data.system_time_sec(),
                  data.monotonic_time_sec(),
                  data.receive_time_sec());
    break;
  }
  case tkm::msg::monitor::Data_What_ContextInfo: {
    tkm::msg::monitor::ContextInfo ctxInfo;
    data.payload().UnpackTo(&ctxInfo);
    writeContextInfo(rq.args.at(Defaults::Arg::SessionHash),
                     ctxInfo,
                     data.system_time_sec(),
                     data.monotonic_time_sec(),
                     data.receive_time_sec());
    break;
  }
  case tkm::msg::monitor::Data_What_SysProcStat: {
    tkm::msg::monitor::SysProcStat sysProcStat;
    data.payload().UnpackTo(&sysProcStat);
    writeSysProcStat(rq.args.at(Defaults::Arg::SessionHash),
                     sysProcStat,
                     data.system_time_sec(),
                     data.monotonic_time_sec(),
                     data.receive_time_sec());
    break;
  }
  case tkm::msg::monitor::Data_What_SysProcBuddyInfo: {
    tkm::msg::monitor::SysProcBuddyInfo sysProcBuddyInfo;
    data.payload().UnpackTo(&sysProcBuddyInfo);
    writeSysProcBuddyInfo(rq.args.at(Defaults::Arg::SessionHash),
                          sysProcBuddyInfo,
                          data.system_time_sec(),
                          data.monotonic_time_sec(),
                          data.receive_time_sec());
    break;
  }
  case tkm::msg::monitor::Data_What_SysProcWireless: {
    tkm::msg::monitor::SysProcWireless sysProcWireless;
    data.payload().UnpackTo(&sysProcWireless);
    writeSysProcWireless(rq.args.at(Defaults::Arg::SessionHash),
                         sysProcWireless,
                         data.system_time_sec(),
                         data.monotonic_time_sec(),
                         data.receive_time_sec());
    break;
  }
  case tkm::msg::monitor::Data_What_SysProcMemInfo: {
    tkm::msg::monitor::SysProcMemInfo sysProcMem;
    data.payload().UnpackTo(&sysProcMem);
    writeSysProcMemInfo(rq.args.at(Defaults::Arg::SessionHash),
                        sysProcMem,
                        data.system_time_sec(),
                        data.monotonic_time_sec(),
                        data.receive_time_sec());
    break;
  }
  case tkm::msg::monitor::Data_What_SysProcPressure: {
    tkm::msg::monitor::SysProcPressure sysProcPressure;
    data.payload().UnpackTo(&sysProcPressure);
    writeSysProcPressure(rq.args.at(Defaults::Arg::SessionHash),
                         sysProcPressure,
                         data.system_time_sec(),
                         data.monotonic_time_sec(),
                         data.receive_time_sec());
    break;
  }
  case tkm::msg::monitor::Data_What_SysProcDiskStats: {
    tkm::msg::monitor::SysProcDiskStats sysProcDiskStats;
    data.payload().UnpackTo(&sysProcDiskStats);
    writeSysProcDiskStats(rq.args.at(Defaults::Arg::SessionHash),
                          sysProcDiskStats,
                          data.system_time_sec(),
                          data.monotonic_time_sec(),
                          data.receive_time_sec());
    break;
  }
  case tkm::msg::monitor::Data_What_SysProcVMStat: {
    tkm::msg::monitor::SysProcVMStat sysProcVMStat;
    data.payload().UnpackTo(&sysProcVMStat);
    writeSysProcVMStat(rq.args.at(Defaults::Arg::SessionHash),
                        sysProcVMStat,
                        data.system_time_sec(),
                        data.monotonic_time_sec(),
                        data.receive_time_sec());
    break;
  }
  default:
    break;
  }

  return true;
}

static bool doConnect(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq)
{
  // No need for DB connect with SQLite
  static_cast<void>(db); // UNUSED
  static_cast<void>(rq); // UNUSED
  return true;
}

static bool doDisconnect(const std::shared_ptr<SQLiteDatabase> db, const IDatabase::Request &rq)
{
  // No need for DB disconnect with SQLite
  static_cast<void>(db); // UNUSED
  static_cast<void>(rq); // UNUSED
  return true;
}

} // namespace tkm::collector

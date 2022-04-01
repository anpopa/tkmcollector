#include "SQLiteDatabase.h"
#include "Application.h"
#include "Defaults.h"
#include "Query.h"

#include "Collector.pb.h"

#include <Envelope.pb.h>
#include <Helpers.h>
#include <any>
#include <filesystem>
#include <string>
#include <vector>

using std::shared_ptr;
using std::string;
namespace fs = std::filesystem;

namespace tkm::collector {

static auto sqlite_callback(void *data, int argc, char **argv, char **colname)
    -> int;
static auto doCheckDatabase(const shared_ptr<SQLiteDatabase> &db,
                            const IDatabase::Request &rq) -> bool;
static auto doInitDatabase(const shared_ptr<SQLiteDatabase> &db,
                           const IDatabase::Request &rq) -> bool;
static auto doGetDevices(const shared_ptr<SQLiteDatabase> &db,
                         const IDatabase::Request &rq) -> bool;
static auto doAddDevice(const shared_ptr<SQLiteDatabase> &db,
                        const IDatabase::Request &rq) -> bool;
static auto doRemoveDevice(const shared_ptr<SQLiteDatabase> &db,
                           const IDatabase::Request &rq) -> bool;
static auto doConnect(const shared_ptr<SQLiteDatabase> &db,
                      const IDatabase::Request &rq) -> bool;
static auto doDisconnect(const shared_ptr<SQLiteDatabase> &db,
                         const IDatabase::Request &rq) -> bool;
static auto doStartDeviceSession(const shared_ptr<SQLiteDatabase> &db,
                                 const IDatabase::Request &rq) -> bool;
static auto doStopDeviceSession(const shared_ptr<SQLiteDatabase> &db,
                                const IDatabase::Request &rq) -> bool;
static auto doGetEntries(const shared_ptr<SQLiteDatabase> &db,
                         const IDatabase::Request &rq) -> bool;

SQLiteDatabase::SQLiteDatabase() : IDatabase() {
  fs::path addr(
      CollectorApp()->getOptions()->getFor(Options::Key::DBServerAddress));
  logDebug() << "Using DB file: " << addr.string();
  if (sqlite3_open(addr.c_str(), &m_db) != SQLITE_OK) {
    sqlite3_close(m_db);
    throw std::runtime_error(sqlite3_errmsg(m_db));
  }
}

SQLiteDatabase::~SQLiteDatabase() { sqlite3_close(m_db); }

void SQLiteDatabase::enableEvents() {
  CollectorApp()->addEventSource(m_queue);

  IDatabase::Request dbrq{.action = IDatabase::Action::CheckDatabase};
  pushRequest(dbrq);
}

auto SQLiteDatabase::runQuery(const std::string &sql,
                              SQLiteDatabase::Query &query) -> bool {
  char *queryError = nullptr;

  logDebug() << "Run query: " << sql;
  if (::sqlite3_exec(m_db, sql.c_str(), sqlite_callback, &query, &queryError) !=
      SQLITE_OK) {
    logError() << "SQLiteDatabase query error: " << queryError;
    sqlite3_free(queryError);
    return false;
  }

  return true;
}

static auto sqlite_callback(void *data, int argc, char **argv, char **colname)
    -> int {
  auto *query = static_cast<SQLiteDatabase::Query *>(data);

  switch (query->type) {
  case SQLiteDatabase::QueryType::Check:
  case SQLiteDatabase::QueryType::Create:
  case SQLiteDatabase::QueryType::DropTables:
    break;
  case SQLiteDatabase::QueryType::GetDevices: {
    auto pld =
        static_cast<std::vector<tkm::msg::collector::DeviceData> *>(query->raw);
    tkm::msg::collector::DeviceData device{};
    for (int i = 0; i < argc; i++) {
      // TODO: Add device data
    }
    pld->emplace_back(device);
    break;
  }
  default:
    logError() << "Unknown query type";
    break;
  }

  return 0;
}

auto SQLiteDatabase::requestHandler(const Request &rq) -> bool {
  switch (rq.action) {
  case IDatabase::Action::CheckDatabase:
    return doCheckDatabase(getShared(), rq);
  case IDatabase::Action::InitDatabase:
    return doInitDatabase(getShared(), rq);
  case IDatabase::Action::Connect:
    return doConnect(getShared(), rq);
  case IDatabase::Action::Disconnect:
    return doDisconnect(getShared(), rq);
  case IDatabase::Action::GetDevices:
    return doGetDevices(getShared(), rq);
  case IDatabase::Action::AddDevice:
    return doAddDevice(getShared(), rq);
  case IDatabase::Action::RemoveDevice:
    return doRemoveDevice(getShared(), rq);
  default:
    break;
  }
  logError() << "Unknown action request";
  return false;
}

static auto doCheckDatabase(const shared_ptr<SQLiteDatabase> &db,
                            const SQLiteDatabase::Request &rq) -> bool {
  // TODO: Handle database check
  return true;
}

static auto doInitDatabase(const shared_ptr<SQLiteDatabase> &db,
                           const SQLiteDatabase::Request &rq) -> bool {
  Dispatcher::Request mrq{.client = rq.client,
                          .action = Dispatcher::Action::SendStatus};

  logDebug() << "Handling DB init request";

  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) ==
        tkmDefaults.valFor(Defaults::Val::True)) {
      SQLiteDatabase::Query query{.type =
                                      SQLiteDatabase::QueryType::DropTables};
      db->runQuery(tkmQuery.dropTables(Query::Type::SQLite3), query);
    }
  }

  SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::Create};
  auto status =
      db->runQuery(tkmQuery.createTables(Query::Type::SQLite3), query);

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId,
                     rq.args.at(Defaults::Arg::RequestId));
  }
  mrq.args.emplace(Defaults::Arg::Status,
                   status == true
                       ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                       : tkmDefaults.valFor(Defaults::Val::StatusError));
  mrq.args.emplace(Defaults::Arg::Reason,
                   status == true ? "Database init complete"
                                  : "Database init failed. Query error");

  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doGetDevices(const shared_ptr<SQLiteDatabase> &db,
                         const IDatabase::Request &rq) -> bool {
  Dispatcher::Request mrq{.client = rq.client,
                          .action = Dispatcher::Action::SendStatus};

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId,
                     rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB GetDevices request from client: "
             << rq.client->getName();

  SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::GetDevices};
  auto queryDevList = std::vector<tkm::msg::collector::DeviceData>();
  query.raw = &queryDevList;

  auto status = db->runQuery(tkmQuery.getDevices(Query::Type::SQLite3), query);
  if (status) {
    tkm::msg::Envelope envelope;
    tkm::msg::collector::Message message;
    tkm::msg::collector::DeviceList devList;

    for (auto &dev : queryDevList) {
      auto tmpDev = devList.add_device();
      tmpDev->CopyFrom(dev);
    }

    // As response to client registration request we ask client to send
    // descriptor
    message.set_type(tkm::msg::collector::Message_Type_DeviceList);
    message.mutable_data()->PackFrom(devList);
    envelope.mutable_mesg()->PackFrom(message);

    envelope.set_target(msg::Envelope_Recipient_Any);
    envelope.set_origin(msg::Envelope_Recipient_Collector);

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
                   status == true
                       ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                       : tkmDefaults.valFor(Defaults::Val::StatusError));

  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doAddDevice(const shared_ptr<SQLiteDatabase> &db,
                        const IDatabase::Request &rq) -> bool {
  Dispatcher::Request mrq{.client = rq.client,
                          .action = Dispatcher::Action::SendStatus};
  auto devId = -1;

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId,
                     rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB AddUser request from client: "
             << rq.client->getName();
  const auto &deviceData =
      std::any_cast<tkm::msg::collector::DeviceData>(rq.bulkData);

  SQLiteDatabase::Query queryCheckExisting{
      .type = SQLiteDatabase::QueryType::GetDeviceById};
  queryCheckExisting.raw = &devId;
  auto status = db->runQuery(
      tkmQuery.getDeviceById(Query::Type::SQLite3, deviceData.id()),
      queryCheckExisting);
  if (status) {
    if (rq.args.count(Defaults::Arg::Forced)) {
      if (rq.args.at(Defaults::Arg::Forced) ==
          tkmDefaults.valFor(Defaults::Val::True)) {
        SQLiteDatabase::Query query{.type =
                                        SQLiteDatabase::QueryType::RemDevice};
        db->runQuery(tkmQuery.remDevice(Query::Type::SQLite3, deviceData.id()),
                     query);
      }
    } else if (devId != -1) {
      mrq.args.emplace(Defaults::Arg::Status,
                       tkmDefaults.valFor(Defaults::Val::StatusError));
      mrq.args.emplace(Defaults::Arg::Reason, "Device already exists");
      return CollectorApp()->getDispatcher()->pushRequest(mrq);
    }

    SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::AddDevice};
    status =
        db->runQuery(tkmQuery.addDevice(Query::Type::SQLite3, deviceData.id(),
                                        deviceData.name(), deviceData.address(),
                                        deviceData.port(), deviceData.state()),
                     query);
    if (!status) {
      mrq.args.emplace(Defaults::Arg::Reason, "Failed to add device");
    } else {
      mrq.args.emplace(Defaults::Arg::Reason, "Device added");
    }
  } else {
    mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing device");
  }

  mrq.args.emplace(Defaults::Arg::Status,
                   status == true
                       ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                       : tkmDefaults.valFor(Defaults::Val::StatusError));
  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doRemoveDevice(const shared_ptr<SQLiteDatabase> &db,
                           const IDatabase::Request &rq) -> bool {
  Dispatcher::Request mrq{.client = rq.client,
                          .action = Dispatcher::Action::SendStatus};
  auto devId = -1;

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId,
                     rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB RemoveUser request from client: "
             << rq.client->getName();

  SQLiteDatabase::Query queryCheckExisting{
      .type = SQLiteDatabase::QueryType::GetDeviceById};
  queryCheckExisting.raw = &devId;

  auto status = db->runQuery(
      tkmQuery.getDeviceById(Query::Type::SQLite3,
                             std::stoul(rq.args.at(Defaults::Arg::Id))),
      queryCheckExisting);
  if (status) {
    if (devId == -1) {
      mrq.args.emplace(Defaults::Arg::Status,
                       tkmDefaults.valFor(Defaults::Val::StatusError));
      mrq.args.emplace(Defaults::Arg::Reason, "No such device");
    }

    SQLiteDatabase::Query query{.type = SQLiteDatabase::QueryType::RemDevice};
    status = db->runQuery(
        tkmQuery.remDevice(Query::Type::SQLite3,
                           std::stoul(rq.args.at(Defaults::Arg::Id))),
        query);

    if (!status) {
      mrq.args.emplace(Defaults::Arg::Reason, "Failed to remove device");
    } else {
      mrq.args.emplace(Defaults::Arg::Reason, "Device removed");
    }
  } else {
    mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing user");
  }

  mrq.args.emplace(Defaults::Arg::Status,
                   status == true
                       ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                       : tkmDefaults.valFor(Defaults::Val::StatusError));
  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doConnect(const shared_ptr<SQLiteDatabase> &db,
                      const IDatabase::Request &rq) -> bool {
  return true;
}

static auto doDisconnect(const shared_ptr<SQLiteDatabase> &db,
                         const IDatabase::Request &rq) -> bool {
  return true;
}

} // namespace tkm::collector

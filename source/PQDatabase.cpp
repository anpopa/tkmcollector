/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     PQDatabase Class
 * @details   PostgreSQL database implementation
 *-
 */

#include "PQDatabase.h"
#include "Application.h"
#include "Defaults.h"
#include "Query.h"

#include "Control.pb.h"
#include "Monitor.pb.h"

#include <Envelope.pb.h>
#include <Helpers.h>
#include <any>
#include <filesystem>
#include <string>
#include <vector>

using std::shared_ptr;
using std::string;

namespace tkm::collector
{

static bool doCheckDatabase(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doInitDatabase(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doLoadDevices(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doGetDevices(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doGetSessions(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doAddDevice(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doRemoveDevice(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doConnect(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doDisconnect(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doStartDeviceSession(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doStopDeviceSession(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doGetEntries(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doAddSession(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doRemSession(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doEndSession(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doCleanSessions(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);
static bool doAddData(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq);

PQDatabase::PQDatabase(std::shared_ptr<Options> options)
: IDatabase(options)
{
  std::stringstream connInfo;

  connInfo << "dbname = " << m_options->getFor(Options::Key::DBName) << " "
           << "user = " << m_options->getFor(Options::Key::DBUserName) << " "
           << "password = " << m_options->getFor(Options::Key::DBUserPassword) << " "
           << "hostaddr = " << m_options->getFor(Options::Key::DBServerAddress) << " "
           << "port = " << m_options->getFor(Options::Key::DBServerPort);

  logDebug() << "Connection string: " << connInfo.str();
  m_connection = std::make_unique<pqxx::connection>(connInfo.str());
  if (m_connection->is_open()) {
    logInfo() << "Opened database successfully: " << m_connection->dbname();
  } else {
    logDebug() << "Can't open database";
    throw std::runtime_error("Fail to open posgress database");
  }
}

bool PQDatabase::reconnect()
{
  if (m_connection->is_open()) {
    return true;
  }
  m_connection.reset();

  std::stringstream connInfo;

  connInfo << "dbname = " << m_options->getFor(Options::Key::DBName) << " "
           << "user = " << m_options->getFor(Options::Key::DBUserName) << " "
           << "password = " << m_options->getFor(Options::Key::DBUserPassword) << " "
           << "hostaddr = " << m_options->getFor(Options::Key::DBServerAddress) << " "
           << "port = " << m_options->getFor(Options::Key::DBServerPort);

  m_connection = std::make_unique<pqxx::connection>(connInfo.str());
  if (m_connection->is_open()) {
    logInfo() << "Opened database successfully: " << m_connection->dbname();
  } else {
    logDebug() << "Can't open database";
    return false;
  }

  return true;
}

auto PQDatabase::runTransaction(const std::string &sql) -> pqxx::result
{
  pqxx::work work(*m_connection);

  auto result = work.exec(sql);
  work.commit();

  return result;
}

void PQDatabase::enableEvents()
{
  CollectorApp()->addEventSource(m_queue);
  IDatabase::Request dbrq{.action = IDatabase::Action::CheckDatabase};
  pushRequest(dbrq);
}

bool PQDatabase::requestHandler(const Request &rq)
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
  case IDatabase::Action::RemSession:
    return doRemSession(getShared(), rq);
  case IDatabase::Action::EndSession:
    return doEndSession(getShared(), rq);
  case IDatabase::Action::CleanSessions:
    return doCleanSessions(getShared(), rq);
  case IDatabase::Action::AddData:
    return doAddData(getShared(), rq);
  default:
    break;
  }
  logError() << "Unknown action request";
  return false;
}

static bool doCheckDatabase(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  logDebug() << "Handling DB check request";
  return true;
}

static bool doInitDatabase(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};
  bool status = true;

  logDebug() << "Handling DB init request";

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  if (!db->getConnection()->is_open()) {
    if (!db->reconnect()) {
      mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
      mrq.args.emplace(Defaults::Arg::Reason, "Database connection error");
      return CollectorApp()->getDispatcher()->pushRequest(mrq);
    }
  }

  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      try {
        db->runTransaction(tkmQuery.dropTables(Query::Type::PostgreSQL));
      } catch (std::exception &e) {
        logError() << "Database query fails: " << e.what();
      }
    }
  }

  try {
    db->runTransaction(tkmQuery.createTables(Query::Type::PostgreSQL));
  } catch (std::exception &e) {
    logError() << "Database query fails: " << e.what();
    status = false;
  }

  mrq.args.emplace(Defaults::Arg::Status,
                   status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                  : tkmDefaults.valFor(Defaults::Val::StatusError));
  mrq.args.emplace(Defaults::Arg::Reason,
                   status == true ? "Database init complete" : "Database init failed. Query error");

  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static bool doLoadDevices(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  auto deviceList = std::vector<tkm::msg::control::DeviceData>();
  bool status = true;

  logDebug() << "Handling DB LoadDevices";

  try {
    auto result = db->runTransaction(tkmQuery.getDevices(Query::Type::PostgreSQL));

    for (pqxx::result::const_iterator c = result.begin(); c != result.end(); ++c) {
      tkm::msg::control::DeviceData deviceData;

      deviceData.set_id(
          c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Id)].as<long>());
      deviceData.set_hash(
          c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Hash)].as<string>());
      deviceData.set_name(
          c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Name)].as<string>());
      deviceData.set_address(
          c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Address)].as<string>());
      deviceData.set_port(
          c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Port)].as<int>());

      deviceList.push_back(deviceData);
    }
  } catch (std::exception &e) {
    logError() << "Database query fails: " << e.what();
    status = false;
  }

  if (status) {
    for (auto &deviceData : deviceList) {
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

static bool doCleanSessions(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  auto sessionList = std::vector<tkm::msg::control::SessionData>();
  bool status = true;

  logDebug() << "Handling DB CleanSessions";

  try {
    auto result = db->runTransaction(tkmQuery.getSessions(Query::Type::PostgreSQL));

    for (pqxx::result::const_iterator c = result.begin(); c != result.end(); ++c) {
      tkm::msg::control::SessionData sessionData;

      sessionData.set_id(
          c[static_cast<pqxx::result::size_type>(Query::SessionColumn::Id)].as<long>());
      sessionData.set_hash(
          c[static_cast<pqxx::result::size_type>(Query::SessionColumn::Hash)].as<string>());
      sessionData.set_name(
          c[static_cast<pqxx::result::size_type>(Query::SessionColumn::Name)].as<string>());
      sessionData.set_started(
          c[static_cast<pqxx::result::size_type>(Query::SessionColumn::StartTimestamp)].as<long>());
      sessionData.set_ended(
          c[static_cast<pqxx::result::size_type>(Query::SessionColumn::EndTimestamp)].as<long>());

      sessionList.push_back(sessionData);
    }
  } catch (std::exception &e) {
    logError() << "Database query fails: " << e.what();
    status = false;
  }

  if (status) {
    for (auto &sessionData : sessionList) {
      if (sessionData.ended() == 0) {
        IDatabase::Request dbrq{.action = IDatabase::Action::EndSession};
        dbrq.args.emplace(Defaults::Arg::SessionHash, sessionData.hash());
        db->pushRequest(dbrq);
      }
    }
  } else {
    logError() << "Failed to clean sessions";
  }

  return true;
}

static bool doGetDevices(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};
  auto deviceList = std::vector<tkm::msg::control::DeviceData>();
  bool status = true;

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB GetDevices request from client: " << rq.client->getName();

  try {
    auto result = db->runTransaction(tkmQuery.getDevices(Query::Type::PostgreSQL));

    for (pqxx::result::const_iterator c = result.begin(); c != result.end(); ++c) {
      tkm::msg::control::DeviceData deviceData;

      deviceData.set_id(
          c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Id)].as<long>());
      deviceData.set_hash(
          c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Hash)].as<string>());
      deviceData.set_name(
          c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Name)].as<string>());
      deviceData.set_address(
          c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Address)].as<string>());
      deviceData.set_port(
          c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Port)].as<int>());

      deviceList.push_back(deviceData);
    }
  } catch (std::exception &e) {
    logError() << "Database query fails: " << e.what();
    status = false;
  }

  if (status) {
    tkm::msg::Envelope envelope;
    tkm::msg::control::Message message;
    tkm::msg::control::DeviceList devList;

    for (auto &dev : deviceList) {
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
  }

  mrq.args.emplace(Defaults::Arg::Status,
                   status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                  : tkmDefaults.valFor(Defaults::Val::StatusError));

  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static bool doGetSessions(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};
  auto sessionList = std::vector<tkm::msg::control::SessionData>();
  bool status = true;

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB GetSessions request from client: " << rq.client->getName();
  const auto &deviceData = std::any_cast<tkm::msg::control::DeviceData>(rq.bulkData);

  try {
    pqxx::result result;

    if (deviceData.hash().empty()) {
      result = db->runTransaction(tkmQuery.getSessions(Query::Type::PostgreSQL));
    } else {
      result = db->runTransaction(tkmQuery.getSessions(Query::Type::PostgreSQL, deviceData.hash()));
    }

    for (pqxx::result::const_iterator c = result.begin(); c != result.end(); ++c) {
      tkm::msg::control::SessionData sessionData;

      sessionData.set_id(
          c[static_cast<pqxx::result::size_type>(Query::SessionColumn::Id)].as<long>());
      sessionData.set_hash(
          c[static_cast<pqxx::result::size_type>(Query::SessionColumn::Hash)].as<string>());
      sessionData.set_name(
          c[static_cast<pqxx::result::size_type>(Query::SessionColumn::Name)].as<string>());
      sessionData.set_started(
          c[static_cast<pqxx::result::size_type>(Query::SessionColumn::StartTimestamp)].as<long>());
      sessionData.set_ended(
          c[static_cast<pqxx::result::size_type>(Query::SessionColumn::EndTimestamp)].as<long>());

      sessionList.push_back(sessionData);
    }
  } catch (std::exception &e) {
    logError() << "Database query fails: " << e.what();
    status = false;
  }

  if (status) {
    tkm::msg::Envelope envelope;
    tkm::msg::control::Message message;
    tkm::msg::control::SessionList msgSessionList;

    for (auto &session : sessionList) {
      if (session.ended() == 0) {
        session.set_state(tkm::msg::control::SessionData_State_Progress);
      } else {
        session.set_state(tkm::msg::control::SessionData_State_Complete);
      }
      auto tmpSession = msgSessionList.add_session();
      tmpSession->CopyFrom(session);
    }

    message.set_type(tkm::msg::control::Message_Type_SessionList);
    message.mutable_data()->PackFrom(msgSessionList);
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
  }

  mrq.args.emplace(Defaults::Arg::Status,
                   status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                  : tkmDefaults.valFor(Defaults::Val::StatusError));

  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static bool doAddDevice(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};
  bool status = true;
  auto devId = -1;

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB AddDevice request from client: " << rq.client->getName();
  const auto &deviceData = std::any_cast<tkm::msg::control::DeviceData>(rq.bulkData);

  try {
    auto result =
        db->runTransaction(tkmQuery.hasDevice(Query::Type::PostgreSQL, deviceData.hash()));
    for (pqxx::result::const_iterator c = result.begin(); c != result.end(); ++c) {
      devId = c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Id)].as<long>();
    }
  } catch (std::exception &e) {
    logError() << "Database query fails: " << e.what();
    status = false;
  }

  if (status) {
    if (rq.args.count(Defaults::Arg::Forced)) {
      if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
        try {
          db->runTransaction(tkmQuery.remDevice(Query::Type::PostgreSQL, deviceData.hash()));
        } catch (std::exception &e) {
          logError() << "Cannot remove device. Database query fails: " << e.what();
          status = false;
        }
      }
    } else if (devId != -1) {
      mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
      mrq.args.emplace(Defaults::Arg::Reason, "Device already exists");
      return CollectorApp()->getDispatcher()->pushRequest(mrq);
    }

    if (status) {
      try {
        db->runTransaction(tkmQuery.addDevice(Query::Type::PostgreSQL,
                                              deviceData.hash(),
                                              deviceData.name(),
                                              deviceData.address(),
                                              deviceData.port()));
      } catch (std::exception &e) {
        logError() << "Database query fails: " << e.what();
        status = false;
      }
    }

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

static bool doRemoveDevice(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};
  bool status = true;
  auto devId = -1;

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB RemoveDevice request from client: " << rq.client->getName();
  const auto &deviceData = std::any_cast<tkm::msg::control::DeviceData>(rq.bulkData);

  try {
    auto result =
        db->runTransaction(tkmQuery.hasDevice(Query::Type::PostgreSQL, deviceData.hash()));
    for (pqxx::result::const_iterator c = result.begin(); c != result.end(); ++c) {
      devId = c[static_cast<pqxx::result::size_type>(Query::DeviceColumn::Id)].as<long>();
    }
  } catch (std::exception &e) {
    logError() << "Database query fails: " << e.what();
    status = false;
  }

  if (status) {
    if (devId == -1) {
      mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
      mrq.args.emplace(Defaults::Arg::Reason, "No such device");
    }

    try {
      db->runTransaction(tkmQuery.remDevice(Query::Type::PostgreSQL, deviceData.hash()));
    } catch (std::exception &e) {
      logError() << "Database query fails: " << e.what();
      status = false;
    }

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

static bool doAddSession(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
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
  try {
    auto result =
        db->runTransaction(tkmQuery.hasSession(Query::Type::PostgreSQL, sessionInfo.hash()));
    for (pqxx::result::const_iterator c = result.begin(); c != result.end(); ++c) {
      sesId = c[static_cast<pqxx::result::size_type>(Query::SessionColumn::Id)].as<long>();
    }
  } catch (std::exception &e) {
    logError() << "Database query fails: " << e.what();
  }
  if (sesId != -1) {
    logError() << "Session hash collision detected. Remove old session " << sessionInfo.hash();
    try {
      db->runTransaction(tkmQuery.remSession(Query::Type::PostgreSQL, sessionInfo.hash()));
    } catch (std::exception &e) {
      logError() << "Failed to remove existing session. Database query fails: " << e.what();
    }
  }

  bool status = true;

  try {
    db->runTransaction(tkmQuery.addSession(
        Query::Type::PostgreSQL, sessionInfo, rq.args.at(Defaults::Arg::DeviceHash), time(NULL)));
  } catch (std::exception &e) {
    logError() << "Database query fails: " << e.what();
    status = false;
  }

  if (!status) {
    logError() << "Query failed to add session";
  }

  return true;
}

static bool doRemSession(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};
  bool status = true;
  auto sesId = -1;

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  logDebug() << "Handling DB RemoveSession request from client: " << rq.client->getName();
  const auto &sessionData = std::any_cast<tkm::msg::control::SessionData>(rq.bulkData);

  try {
    auto result =
        db->runTransaction(tkmQuery.hasSession(Query::Type::PostgreSQL, sessionData.hash()));
    for (pqxx::result::const_iterator c = result.begin(); c != result.end(); ++c) {
      sesId = c[static_cast<pqxx::result::size_type>(Query::SessionColumn::Id)].as<long>();
    }
  } catch (std::exception &e) {
    logError() << "Database query fails: " << e.what();
    status = false;
  }

  if (status) {
    if (sesId == -1) {
      mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
      mrq.args.emplace(Defaults::Arg::Reason, "No such session");
    } else {
      try {
        db->runTransaction(tkmQuery.remSession(Query::Type::PostgreSQL, sessionData.hash()));
      } catch (std::exception &e) {
        logError() << "Database query fails: " << e.what();
        status = false;
      }

      if (!status) {
        mrq.args.emplace(Defaults::Arg::Reason, "Failed to remove session");
      } else {
        mrq.args.emplace(Defaults::Arg::Reason, "Session removed");
      }
    }
  } else {
    mrq.args.emplace(Defaults::Arg::Reason, "Cannot check existing session");
  }

  mrq.args.emplace(Defaults::Arg::Status,
                   status == true ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                  : tkmDefaults.valFor(Defaults::Val::StatusError));
  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static bool doEndSession(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  logDebug() << "Handling DB EndSession request";
  if ((rq.args.count(Defaults::Arg::SessionHash) == 0)) {
    logError() << "Invalid session data";
    throw std::runtime_error("Invalid arguments");
  }

  logDebug() << "Mark end session for " << rq.args.at(Defaults::Arg::SessionHash);
  try {
    db->runTransaction(
        tkmQuery.endSession(Query::Type::PostgreSQL, rq.args.at(Defaults::Arg::SessionHash)));
  } catch (std::exception &e) {
    logError() << "Query failed to mark end session. Database query fails: " << e.what();
  }

  return true;
}

static bool doAddData(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  const auto &data = std::any_cast<tkm::msg::monitor::Data>(rq.bulkData);
  bool status = true;

  if ((rq.args.count(Defaults::Arg::SessionHash) == 0)) {
    logError() << "Invalid session data";
    throw std::runtime_error("Invalid arguments");
  }

  auto writeProcAcct = [&db, &rq, &status](const std::string &sessionHash,
                                           const tkm::msg::monitor::ProcAcct &acct,
                                           uint64_t systemTime,
                                           uint64_t monotonicTime,
                                           uint64_t receiveTime) {
    try {
      db->runTransaction(tkmQuery.addData(
          Query::Type::PostgreSQL, sessionHash, acct, systemTime, monotonicTime, receiveTime));
    } catch (std::exception &e) {
      logError() << "Query failed to addData. Database query fails: " << e.what();
      status = false;
    }
  };

  auto writeProcInfo = [&db, &rq, &status](const std::string &sessionHash,
                                           const tkm::msg::monitor::ProcInfo &info,
                                           uint64_t systemTime,
                                           uint64_t monotonicTime,
                                           uint64_t receiveTime) {
    try {
      db->runTransaction(tkmQuery.addData(
          Query::Type::PostgreSQL, sessionHash, info, systemTime, monotonicTime, receiveTime));
    } catch (std::exception &e) {
      logError() << "Query failed to addData. Database query fails: " << e.what();
      status = false;
    }
  };

  auto writeContextInfo = [&db, &rq, &status](const std::string &sessionHash,
                                              const tkm::msg::monitor::ContextInfo &info,
                                              uint64_t systemTime,
                                              uint64_t monotonicTime,
                                              uint64_t receiveTime) {
    try {
      db->runTransaction(tkmQuery.addData(
          Query::Type::PostgreSQL, sessionHash, info, systemTime, monotonicTime, receiveTime));
    } catch (std::exception &e) {
      logError() << "Query failed to addData. Database query fails: " << e.what();
      status = false;
    }
  };

  auto writeSysProcStat = [&db, &rq, &status](const std::string &sessionHash,
                                              const tkm::msg::monitor::SysProcStat &sysProcStat,
                                              uint64_t systemTime,
                                              uint64_t monotonicTime,
                                              uint64_t receiveTime) {
    try {
      db->runTransaction(tkmQuery.addData(Query::Type::PostgreSQL,
                                          sessionHash,
                                          sysProcStat.cpu(),
                                          systemTime,
                                          monotonicTime,
                                          receiveTime));
    } catch (std::exception &e) {
      logError() << "Query failed to addData. Database query fails: " << e.what();
      status = false;
    }

    if (status) {
      for (const auto &coreStat : sysProcStat.core()) {
        try {
          db->runTransaction(tkmQuery.addData(Query::Type::PostgreSQL,
                                              sessionHash,
                                              coreStat,
                                              systemTime,
                                              monotonicTime,
                                              receiveTime));
        } catch (std::exception &e) {
          logError() << "Query failed to addData. Database query fails: " << e.what();
          status = false;
        }
      }
    }
  };

  auto writeSysProcMemInfo =
      [&db, &rq, &status](const std::string &sessionHash,
                          const tkm::msg::monitor::SysProcMemInfo &sysProcMem,
                          uint64_t systemTime,
                          uint64_t monotonicTime,
                          uint64_t receiveTime) {
        try {
          db->runTransaction(tkmQuery.addData(Query::Type::PostgreSQL,
                                              sessionHash,
                                              sysProcMem,
                                              systemTime,
                                              monotonicTime,
                                              receiveTime));
        } catch (std::exception &e) {
          logError() << "Query failed to addData. Database query fails: " << e.what();
          status = false;
        }
      };

  auto writeSysProcPressure =
      [&db, &rq, &status](const std::string &sessionHash,
                          const tkm::msg::monitor::SysProcPressure &sysProcPressure,
                          uint64_t systemTime,
                          uint64_t monotonicTime,
                          uint64_t receiveTime) {
        try {
          db->runTransaction(tkmQuery.addData(Query::Type::PostgreSQL,
                                              sessionHash,
                                              sysProcPressure,
                                              systemTime,
                                              monotonicTime,
                                              receiveTime));
        } catch (std::exception &e) {
          logError() << "Query failed to addData. Database query fails: " << e.what();
          status = false;
        }
      };

  auto writeSysProcDiskStats =
      [&db, &rq, &status](const std::string &sessionHash,
                          const tkm::msg::monitor::SysProcDiskStats &sysProcDiskStats,
                          uint64_t systemTime,
                          uint64_t monotonicTime,
                          uint64_t receiveTime) {
        try {
          db->runTransaction(tkmQuery.addData(Query::Type::PostgreSQL,
                                              sessionHash,
                                              sysProcDiskStats,
                                              systemTime,
                                              monotonicTime,
                                              receiveTime));
        } catch (std::exception &e) {
          logError() << "Query failed to addData. Database query fails: " << e.what();
          status = false;
        }
      };

  auto writeProcEvent = [&db, &rq, &status](const std::string &sessionHash,
                                            const tkm::msg::monitor::ProcEvent &procEvent,
                                            uint64_t systemTime,
                                            uint64_t monotonicTime,
                                            uint64_t receiveTime) {
    try {
      db->runTransaction(tkmQuery.addData(
          Query::Type::PostgreSQL, sessionHash, procEvent, systemTime, monotonicTime, receiveTime));
    } catch (std::exception &e) {
      logError() << "Query failed to addData. Database query fails: " << e.what();
      status = false;
    }
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
  default:
    break;
  }

  return true;
}

static bool doConnect(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  logDebug() << "Handling DB Connect request";

  if (db->getConnection()->is_open()) {
    return true;
  }

  db->reconnect();

  return true;
}

static bool doDisconnect(const shared_ptr<PQDatabase> &db, const IDatabase::Request &rq)
{
  logDebug() << "Handling DB Disconnect request";
  return true;
}

} // namespace tkm::collector

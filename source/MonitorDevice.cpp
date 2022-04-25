/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     MonitorDevice Class
 * @details   Device object associated with a device entry
 *-
 */

#include <chrono>
#include <ctime>
#include <iostream>
#include <ostream>
#include <string>
#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "Dispatcher.h"
#include "Helpers.h"
#include "IDatabase.h"
#include "MonitorDevice.h"

#include "Collector.pb.h"
#include "Control.pb.h"
#include "Monitor.pb.h"

using std::shared_ptr;
using std::string;

namespace tkm::collector
{
static bool doConnect(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq);
static bool doDisconnect(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq);
static bool doSendDescriptor(const shared_ptr<MonitorDevice> &mgr,
                             const MonitorDevice::Request &rq);
static bool doRequestSession(const shared_ptr<MonitorDevice> &mgr,
                             const MonitorDevice::Request &rq);
static bool doSetSession(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq);
static bool doStartCollecting(const shared_ptr<MonitorDevice> &mgr,
                              const MonitorDevice::Request &rq);
static bool doStopCollecting(const shared_ptr<MonitorDevice> &mgr,
                             const MonitorDevice::Request &rq);
static bool doStartStream(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq);
static bool doStopStream(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq);
static bool doProcessData(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq);
static bool doStatus(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq);

void MonitorDevice::enableEvents()
{
  CollectorApp()->addEventSource(m_queue);
}

bool MonitorDevice::createConnection()
{
  if (m_connection)
    return false;

  m_connection = std::make_shared<Connection>(getShared());
  return true;
}

void MonitorDevice::deleteConnection()
{
  m_connection.reset();
}

void MonitorDevice::enableConnection()
{
  m_connection->enableEvents();
}

bool MonitorDevice::pushRequest(Request &request)
{
  return m_queue->push(request);
}

void MonitorDevice::updateState(tkm::msg::control::DeviceData_State state)
{
  m_deviceData.set_state(state);

  if (state == tkm::msg::control::DeviceData_State_Disconnected) {
    if (m_sessionData.hash().length() > 0) {
      IDatabase::Request dbrq{.action = IDatabase::Action::EndSession};
      dbrq.args.emplace(Defaults::Arg::SessionHash, m_sessionData.hash());
      CollectorApp()->getDatabase()->pushRequest(dbrq);
    }
  }
}

bool MonitorDevice::requestHandler(const Request &request)
{
  switch (request.action) {
  case MonitorDevice::Action::Connect:
    return doConnect(getShared(), request);
  case MonitorDevice::Action::Disconnect:
    return doDisconnect(getShared(), request);
  case MonitorDevice::Action::SendDescriptor:
    return doSendDescriptor(getShared(), request);
  case MonitorDevice::Action::RequestSession:
    return doRequestSession(getShared(), request);
  case MonitorDevice::Action::SetSession:
    return doSetSession(getShared(), request);
  case MonitorDevice::Action::StartCollecting:
    return doStartCollecting(getShared(), request);
  case MonitorDevice::Action::StopCollecting:
    return doStopCollecting(getShared(), request);
  case MonitorDevice::Action::StartStream:
    return doStartStream(getShared(), request);
  case MonitorDevice::Action::StopStream:
    return doStopStream(getShared(), request);
  case MonitorDevice::Action::ProcessData:
    return doProcessData(getShared(), request);
  case MonitorDevice::Action::Status:
    return doStatus(getShared(), request);
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

static bool doConnect(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  if (!mgr->createConnection()) {
    logError() << "Connection to device failed";
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason, "Failed to create connection object");
    return CollectorApp()->getDispatcher()->pushRequest(mrq);
  }

  if (mgr->getConnection()->connect() < 0) {
    // The event loop is not using the shared connection object,
    // just release this connection for MonitorDevice
    mgr->deleteConnection();

    logError() << "Connection to device failed";
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason, "Connection Failed");
    return CollectorApp()->getDispatcher()->pushRequest(mrq);
  }

  // Ready to enable connection event loop source
  mgr->enableConnection();

  MonitorDevice::Request nrq = {.client = rq.client,
                                .action = MonitorDevice::Action::SendDescriptor,
                                .args = rq.args,
                                .bulkData = rq.bulkData};

  return mgr->pushRequest(nrq);
}

static bool doSendDescriptor(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};
  tkm::msg::collector::Descriptor descriptor;
  descriptor.set_id("Collector");

  if (!sendCollectorDescriptor(mgr->getConnection()->getFD(), descriptor)) {
    if (rq.args.count(Defaults::Arg::RequestId)) {
      mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason, "Failed to send descriptor");

    logError() << "Failed to send descriptor";
    return CollectorApp()->getDispatcher()->pushRequest(mrq);
  }

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }
  mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
  mrq.args.emplace(Defaults::Arg::Reason, "Connected");

  // We consider the state to be connected after descriptor was sent
  mgr->updateState(tkm::msg::control::DeviceData_State_Connected);

  logDebug() << "Client connected";
  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static bool doRequestSession(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
{
  tkm::msg::Envelope envelope;
  tkm::msg::collector::Request request;

  request.set_id("CreateSession");
  request.set_type(tkm::msg::collector::Request::Type::Request_Type_CreateSession);

  envelope.mutable_mesg()->PackFrom(request);
  envelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

  logDebug() << "Request session to server";
  return mgr->getConnection()->writeEnvelope(envelope);
}

static bool doSetSession(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
{
  const auto &sessionInfo = std::any_cast<tkm::msg::monitor::SessionInfo>(rq.bulkData);

  // Update our session data
  logDebug() << "Session created: " << sessionInfo.id();
  mgr->getSessionData().set_hash(sessionInfo.id());
  mgr->getSessionData().set_proc_acct_poll_interval(sessionInfo.proc_acct_poll_interval());
  mgr->getSessionData().set_sys_proc_stat_poll_interval(sessionInfo.sys_proc_stat_poll_interval());
  mgr->getSessionData().set_sys_proc_meminfo_poll_interval(
      sessionInfo.sys_proc_meminfo_poll_interval());
  mgr->getSessionData().set_sys_proc_pressure_poll_interval(
      sessionInfo.sys_proc_pressure_poll_interval());
  mgr->updateState(tkm::msg::control::DeviceData_State_SessionSet);

  // Create session
  IDatabase::Request dbrq{.action = IDatabase::Action::AddSession};
  dbrq.args.emplace(Defaults::Arg::DeviceHash, mgr->getDeviceData().hash());
  dbrq.args.emplace(Defaults::Arg::SessionHash, mgr->getSessionData().hash());
  CollectorApp()->getDatabase()->pushRequest(dbrq);

  // Start data stream
  MonitorDevice::Request nrq = {.action = MonitorDevice::Action::StartStream};
  return mgr->pushRequest(nrq);
}

static bool doDisconnect(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  if (mgr->getDeviceData().state() != tkm::msg::control::DeviceData_State_Disconnected) {
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
    mrq.args.emplace(Defaults::Arg::Reason, "Device disconnected");

    // Remove the associated connection event source
    // This will trigger device deleteConnection() and socket cleanup
    CollectorApp()->remEventSource(mgr->getConnection()->getShared());
  } else {
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason, "Device not connected");
  }

  logDebug() << mrq.args.at(Defaults::Arg::Reason);
  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static bool doStartStream(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &)
{
  // ProcAcct timer
  if (mgr->getProcAcctTimer() != nullptr) {
    if (mgr->getProcAcctTimer()->isArmed()) {
      mgr->getProcAcctTimer()->stop();
    }
    mgr->getProcAcctTimer().reset();
    mgr->getProcAcctTimer() = nullptr;
  }
  mgr->getProcAcctTimer() = std::make_shared<Timer>("ProcAcctTimer", [mgr]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request ProcAcct from " << mgr->getDeviceData().name();

    requestMessage.set_id("GetProcAcct");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetProcAcct);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return mgr->getConnection()->writeEnvelope(requestEnvelope);
  });
  mgr->getProcAcctTimer()->start(mgr->getSessionData().proc_acct_poll_interval(), true);
  CollectorApp()->addEventSource(mgr->getProcAcctTimer());

  // SysProcStat timer
  if (mgr->getSysProcStatTimer() != nullptr) {
    if (mgr->getSysProcStatTimer()->isArmed()) {
      mgr->getSysProcStatTimer()->stop();
    }
    mgr->getSysProcStatTimer().reset();
    mgr->getSysProcStatTimer() = nullptr;
  }
  mgr->getSysProcStatTimer() = std::make_shared<Timer>("SysProcStatTimer", [mgr]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request SysProcStat from " << mgr->getDeviceData().name();

    requestMessage.set_id("GetSysProcStat");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetSysProcStat);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return mgr->getConnection()->writeEnvelope(requestEnvelope);
  });
  mgr->getSysProcStatTimer()->start(mgr->getSessionData().sys_proc_stat_poll_interval(), true);
  CollectorApp()->addEventSource(mgr->getSysProcStatTimer());

  // SysProcMemInfo timer
  if (mgr->getSysProcMemInfoTimer() != nullptr) {
    if (mgr->getSysProcMemInfoTimer()->isArmed()) {
      mgr->getSysProcMemInfoTimer()->stop();
    }
    mgr->getSysProcMemInfoTimer().reset();
    mgr->getSysProcMemInfoTimer() = nullptr;
  }
  mgr->getSysProcMemInfoTimer() = std::make_shared<Timer>("SysProcMemInfoTimer", [mgr]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request SysProcMemInfo from " << mgr->getDeviceData().name();

    requestMessage.set_id("GetSysProcMemInfo");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetSysProcMeminfo);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return mgr->getConnection()->writeEnvelope(requestEnvelope);
  });
  mgr->getSysProcMemInfoTimer()->start(mgr->getSessionData().sys_proc_meminfo_poll_interval(),
                                       true);
  CollectorApp()->addEventSource(mgr->getSysProcMemInfoTimer());

  // SysProcPressure timer
  if (mgr->getSysProcPressureTimer() != nullptr) {
    if (mgr->getSysProcPressureTimer()->isArmed()) {
      mgr->getSysProcPressureTimer()->stop();
    }
    mgr->getSysProcPressureTimer().reset();
    mgr->getSysProcPressureTimer() = nullptr;
  }
  mgr->getSysProcPressureTimer() = std::make_shared<Timer>("SysProcPressureTimer", [mgr]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request SysProcPressure from " << mgr->getDeviceData().name();

    requestMessage.set_id("GetSysProcPressure");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetSysProcPressure);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return mgr->getConnection()->writeEnvelope(requestEnvelope);
  });
  mgr->getSysProcPressureTimer()->start(mgr->getSessionData().sys_proc_pressure_poll_interval(),
                                        true);
  CollectorApp()->addEventSource(mgr->getSysProcPressureTimer());

  mgr->getDeviceData().set_state(tkm::msg::control::DeviceData_State_Collecting);

  return true;
}

static bool doStopStream(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &)
{
  if (mgr->getProcAcctTimer() != nullptr) {
    if (mgr->getProcAcctTimer()->isArmed()) {
      mgr->getProcAcctTimer()->stop();
    }
    mgr->getProcAcctTimer().reset();
    mgr->getProcAcctTimer() = nullptr;
  }
  if (mgr->getSysProcStatTimer() != nullptr) {
    if (mgr->getSysProcStatTimer()->isArmed()) {
      mgr->getSysProcStatTimer()->stop();
    }
    mgr->getSysProcStatTimer().reset();
    mgr->getSysProcStatTimer() = nullptr;
  }
  if (mgr->getSysProcMemInfoTimer() != nullptr) {
    if (mgr->getSysProcMemInfoTimer()->isArmed()) {
      mgr->getSysProcMemInfoTimer()->stop();
    }
    mgr->getSysProcMemInfoTimer().reset();
    mgr->getSysProcMemInfoTimer() = nullptr;
  }
  if (mgr->getSysProcPressureTimer() != nullptr) {
    if (mgr->getSysProcPressureTimer()->isArmed()) {
      mgr->getSysProcPressureTimer()->stop();
    }
    mgr->getSysProcPressureTimer().reset();
    mgr->getSysProcPressureTimer() = nullptr;
  }

  mgr->getDeviceData().set_state(tkm::msg::control::DeviceData_State_Idle);

  return true;
}

static bool doStartCollecting(const shared_ptr<MonitorDevice> &mgr,
                              const MonitorDevice::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  if ((mgr->getDeviceData().state() == tkm::msg::control::DeviceData_State_Connected) ||
      (mgr->getDeviceData().state() == tkm::msg::control::DeviceData_State_Idle)) {
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
    mrq.args.emplace(Defaults::Arg::Reason, "Collecting requested");

    MonitorDevice::Request nrq = {.action = MonitorDevice::Action::RequestSession};
    mgr->pushRequest(nrq);
  } else if (mgr->getDeviceData().state() == tkm::msg::control::DeviceData_State_SessionSet) {
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
    mrq.args.emplace(Defaults::Arg::Reason, "Collecting start requested");

    MonitorDevice::Request nrq = {.action = MonitorDevice::Action::StartStream};
    mgr->pushRequest(nrq);
  } else {
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason, "Device not connected");
  }

  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static bool doStopCollecting(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
{
  Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};

  if (rq.args.count(Defaults::Arg::RequestId)) {
    mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
  }

  if (mgr->getDeviceData().state() == tkm::msg::control::DeviceData_State_Collecting) {
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
    mrq.args.emplace(Defaults::Arg::Reason, "Collecting stop requested");

    MonitorDevice::Request nrq = {.action = MonitorDevice::Action::StopStream};
    mgr->pushRequest(nrq);
  } else {
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason, "Device not streaming");
  }

  return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static bool doProcessData(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
{
  // Add entry to database
  IDatabase::Request dbrq{.action = IDatabase::Action::AddData, .bulkData = rq.bulkData};
  dbrq.args.emplace(Defaults::Arg::SessionHash, mgr->getSessionData().hash());
  return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static bool doStatus(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
{
  const auto &serverStatus = std::any_cast<tkm::msg::monitor::Status>(rq.bulkData);
  std::string what;

  switch (serverStatus.what()) {
  case tkm::msg::monitor::Status_What_OK:
    what = tkmDefaults.valFor(Defaults::Val::StatusOkay);
    break;
  case tkm::msg::monitor::Status_What_Busy:
    what = tkmDefaults.valFor(Defaults::Val::StatusBusy);
    break;
  case tkm::msg::monitor::Status_What_Error:
  default:
    what = tkmDefaults.valFor(Defaults::Val::StatusError);
    break;
  }

  logDebug() << "Server status (" << serverStatus.request_id() << "): " << what
             << " Reason: " << serverStatus.reason();

  return true;
}

} // namespace tkm::collector

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
static bool doConnect(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq);
static bool doDisconnect(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq);
static bool doSendDescriptor(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq);
static bool doRequestSession(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq);
static bool doSetSession(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq);
static bool doStartCollecting(const shared_ptr<MonitorDevice> mgr,
                              const MonitorDevice::Request &rq);
static bool doStopCollecting(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq);
static bool doStartStream(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq);
static bool doStopStream(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq);
static bool doProcessData(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq);
static bool doStatus(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq);

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
  stopUpdateLanes();
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

void MonitorDevice::startUpdateLanes(void)
{
  m_fastLaneTimer = std::make_shared<Timer>("FastLaneTimer", [this]() {
    m_dataSources.foreach ([](const std::shared_ptr<DataSource> &entry) {
      if ((entry->getUpdateLane() == DataSource::UpdateLane::Fast) ||
          (entry->getUpdateLane() == DataSource::UpdateLane::Any)) {
        entry->update();
      }
    });
    return true;
  });

  m_paceLaneTimer = std::make_shared<Timer>("PaceLaneTimer", [this]() {
    m_dataSources.foreach ([](const std::shared_ptr<DataSource> &entry) {
      if ((entry->getUpdateLane() == DataSource::UpdateLane::Pace) ||
          (entry->getUpdateLane() == DataSource::UpdateLane::Any)) {
        entry->update();
      }
    });
    return true;
  });

  m_slowLaneTimer = std::make_shared<Timer>("SlowLaneTimer", [this]() {
    m_dataSources.foreach ([](const std::shared_ptr<DataSource> &entry) {
      if ((entry->getUpdateLane() == DataSource::UpdateLane::Slow) ||
          (entry->getUpdateLane() == DataSource::UpdateLane::Any)) {
        entry->update();
      }
    });
    return true;
  });

  configUpdateLanes();

  m_fastLaneTimer->start(getSessionInfo().fast_lane_interval(), true);
  m_paceLaneTimer->start(getSessionInfo().pace_lane_interval(), true);
  m_slowLaneTimer->start(getSessionInfo().slow_lane_interval(), true);

  CollectorApp()->addEventSource(m_fastLaneTimer);
  CollectorApp()->addEventSource(m_paceLaneTimer);
  CollectorApp()->addEventSource(m_slowLaneTimer);
}

void MonitorDevice::stopUpdateLanes(void)
{
  if (m_fastLaneTimer != nullptr) {
    m_fastLaneTimer->stop();
    CollectorApp()->remEventSource(m_fastLaneTimer);
  }

  if (m_paceLaneTimer != nullptr) {
    m_paceLaneTimer->stop();
    CollectorApp()->remEventSource(m_paceLaneTimer);
  }

  if (m_slowLaneTimer != nullptr) {
    m_slowLaneTimer->stop();
    CollectorApp()->remEventSource(m_slowLaneTimer);
  }
}

void MonitorDevice::configUpdateLanes(void)
{
  const auto procAcctUpdateCallback = [this]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request ProcAcct data to " << getDeviceData().name();

    requestMessage.set_id("GetProcAcct");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetProcAcct);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return getConnection()->writeEnvelope(requestEnvelope);
  };

  const auto procInfoUpdateCallback = [this]() -> bool {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request ProcInfo data to " << getDeviceData().name();

    requestMessage.set_id("GetProcInfo");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetProcInfo);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return getConnection()->writeEnvelope(requestEnvelope);
  };

  const auto contextInfoUpdateCallback = [this]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request ContextInfo data to " << getDeviceData().name();

    requestMessage.set_id("GetContextInfo");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetContextInfo);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return getConnection()->writeEnvelope(requestEnvelope);
  };

  const auto procEventUpdateCallback = [this]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request ProcEvent data to " << getDeviceData().name();

    requestMessage.set_id("GetProcEvent");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetProcEventStats);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return getConnection()->writeEnvelope(requestEnvelope);
  };

  const auto sysProcStatUpdateCallback = [this]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request SysProcStat data to " << getDeviceData().name();

    requestMessage.set_id("GetSysProcStat");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetSysProcStat);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return getConnection()->writeEnvelope(requestEnvelope);
  };

  const auto sysProcBuddyInfoUpdateCallback = [this]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request SysProcBuddyInfo data to " << getDeviceData().name();

    requestMessage.set_id("GetSysBuddyInfo");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetSysProcBuddyInfo);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return getConnection()->writeEnvelope(requestEnvelope);
  };

  const auto sysProcWirelessUpdateCallback = [this]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request SysProcWireless data to " << getDeviceData().name();

    requestMessage.set_id("GetSysProcWireless");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetSysProcWireless);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return getConnection()->writeEnvelope(requestEnvelope);
  };

  const auto sysProcMemInfoUpdateCallback = [this]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request SysProcMemInfo data to " << getDeviceData().name();

    requestMessage.set_id("GetSysProcMemInfo");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetSysProcMemInfo);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return getConnection()->writeEnvelope(requestEnvelope);
  };

  const auto sysProcDiskStatsUpdateCallback = [this]() {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request SysProcDiskStats data to " << getDeviceData().name();

    requestMessage.set_id("GetSysProcDiskStats");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetSysProcDiskStats);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return getConnection()->writeEnvelope(requestEnvelope);
  };

  const auto sysProcPressureUpdateCallback = [this]() -> bool {
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    logDebug() << "Request SysProcPressure data to " << getDeviceData().name();

    requestMessage.set_id("GetSysProcPressure");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetSysProcPressure);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    return getConnection()->writeEnvelope(requestEnvelope);
  };

  // Clear the existing list
  m_dataSources.foreach (
      [this](const std::shared_ptr<DataSource> &entry) { m_dataSources.remove(entry); });
  m_dataSources.commit();

  for (const auto &dataSourceType : m_sessionInfo.fast_lane_sources()) {
    switch (dataSourceType) {
    case msg::monitor::SessionInfo_DataSource_ProcInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "ProcInfo", DataSource::UpdateLane::Fast, procInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_ProcAcct:
      m_dataSources.append(std::make_shared<DataSource>(
          "ProcAcct", DataSource::UpdateLane::Fast, procAcctUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_ProcEvent:
      m_dataSources.append(std::make_shared<DataSource>(
          "ProcEvent", DataSource::UpdateLane::Fast, procEventUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_ContextInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "ContextInfo", DataSource::UpdateLane::Fast, contextInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcStat:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcStat", DataSource::UpdateLane::Fast, sysProcStatUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcBuddyInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcBuddyInfo", DataSource::UpdateLane::Fast, sysProcBuddyInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcWireless:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcWireless", DataSource::UpdateLane::Fast, sysProcWirelessUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcMemInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcMemInfo", DataSource::UpdateLane::Fast, sysProcMemInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcPressure:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcPressure", DataSource::UpdateLane::Fast, sysProcPressureUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcDiskStats:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcDiskStats", DataSource::UpdateLane::Fast, sysProcDiskStatsUpdateCallback));
      break;
    default:
      break;
    }
  }

  for (const auto &dataSourceType : m_sessionInfo.pace_lane_sources()) {
    switch (dataSourceType) {
    case msg::monitor::SessionInfo_DataSource_ProcInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "ProcInfo", DataSource::UpdateLane::Pace, procInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_ProcAcct:
      m_dataSources.append(std::make_shared<DataSource>(
          "ProcAcct", DataSource::UpdateLane::Pace, procAcctUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_ProcEvent:
      m_dataSources.append(std::make_shared<DataSource>(
          "ProcEvent", DataSource::UpdateLane::Pace, procEventUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_ContextInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "ContextInfo", DataSource::UpdateLane::Pace, contextInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcStat:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcStat", DataSource::UpdateLane::Pace, sysProcStatUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcBuddyInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcBuddyInfo", DataSource::UpdateLane::Pace, sysProcBuddyInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcWireless:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcWireless", DataSource::UpdateLane::Pace, sysProcWirelessUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcMemInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcMemInfo", DataSource::UpdateLane::Pace, sysProcMemInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcPressure:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcPressure", DataSource::UpdateLane::Pace, sysProcPressureUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcDiskStats:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcDiskStats", DataSource::UpdateLane::Pace, sysProcDiskStatsUpdateCallback));
      break;
    default:
      break;
    }
  }

  for (const auto &dataSourceType : m_sessionInfo.slow_lane_sources()) {
    switch (dataSourceType) {
    case msg::monitor::SessionInfo_DataSource_ProcInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "ProcInfo", DataSource::UpdateLane::Slow, procInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_ProcAcct:
      m_dataSources.append(std::make_shared<DataSource>(
          "ProcAcct", DataSource::UpdateLane::Slow, procAcctUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_ProcEvent:
      m_dataSources.append(std::make_shared<DataSource>(
          "ProcEvent", DataSource::UpdateLane::Slow, procEventUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_ContextInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "ContextInfo", DataSource::UpdateLane::Slow, contextInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcStat:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcStat", DataSource::UpdateLane::Slow, sysProcStatUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcBuddyInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcBuddyInfo", DataSource::UpdateLane::Slow, sysProcBuddyInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcWireless:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcWireless", DataSource::UpdateLane::Slow, sysProcWirelessUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcMemInfo:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcMemInfo", DataSource::UpdateLane::Slow, sysProcMemInfoUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcPressure:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcPressure", DataSource::UpdateLane::Slow, sysProcPressureUpdateCallback));
      break;
    case msg::monitor::SessionInfo_DataSource_SysProcDiskStats:
      m_dataSources.append(std::make_shared<DataSource>(
          "SysProcDiskStats", DataSource::UpdateLane::Slow, sysProcDiskStatsUpdateCallback));
      break;
    default:
      break;
    }
  }

  m_dataSources.commit();
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

static bool doConnect(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq)
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

static bool doSendDescriptor(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq)
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

static bool doRequestSession(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq)
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

static bool doSetSession(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq)
{
  const auto &sessionInfo = std::any_cast<tkm::msg::monitor::SessionInfo>(rq.bulkData);

  // Update our session data
  logDebug() << "Session created: " << sessionInfo.hash()
             << " FastLaneInterval=" << sessionInfo.fast_lane_interval()
             << " PaceLaneInterval=" << sessionInfo.pace_lane_interval()
             << " SlowLameInterval=" << sessionInfo.slow_lane_interval();

  mgr->getSessionInfo().CopyFrom(sessionInfo);
  mgr->getSessionData().set_hash(sessionInfo.hash());
  mgr->updateState(tkm::msg::control::DeviceData_State_SessionSet);

  // Create session
  IDatabase::Request dbrq{.action = IDatabase::Action::AddSession, .bulkData = sessionInfo};
  dbrq.args.emplace(Defaults::Arg::DeviceHash, mgr->getDeviceData().hash());
  CollectorApp()->getDatabase()->pushRequest(dbrq);

  // Start data stream
  MonitorDevice::Request nrq = {.action = MonitorDevice::Action::StartStream};
  return mgr->pushRequest(nrq);
}

static bool doDisconnect(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq)
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

static bool doStartStream(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &)
{
  mgr->getDeviceData().set_state(tkm::msg::control::DeviceData_State_Collecting);
  mgr->startUpdateLanes();
  return true;
}

static bool doStopStream(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &)
{
  mgr->stopUpdateLanes();
  mgr->getDeviceData().set_state(tkm::msg::control::DeviceData_State_Idle);
  return true;
}

static bool doStartCollecting(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq)
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

static bool doStopCollecting(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq)
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

static bool doProcessData(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq)
{
  // Add entry to database
  IDatabase::Request dbrq{.action = IDatabase::Action::AddData, .bulkData = rq.bulkData};
  dbrq.args.emplace(Defaults::Arg::SessionHash, mgr->getSessionData().hash());
  return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static bool doStatus(const shared_ptr<MonitorDevice> mgr, const MonitorDevice::Request &rq)
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

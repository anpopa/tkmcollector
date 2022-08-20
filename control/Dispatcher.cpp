/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Dispatcher Class
 * @details   Main application event dispatcher
 *-
 */

#include <unistd.h>

#include "Application.h"
#include "Dispatcher.h"
#include "Helpers.h"

namespace tkm::control
{

static bool doConnect(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doSendDescriptor(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doRequestSession(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doSetSession(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doQuit(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doInitDatabase(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doGetDevices(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doGetSessions(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doRemoveSession(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doAddDevice(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doRemoveDevice(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doConnectDevice(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doDisconnectDevice(const std::shared_ptr<Dispatcher> mgr,
                               const Dispatcher::Request &rq);
static bool doStartCollecting(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doStopCollecting(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doQuitCollector(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doCollectorStatus(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doDeviceList(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doSessionList(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);

void Dispatcher::enableEvents()
{
  ControlApp()->addEventSource(m_queue);
}

bool Dispatcher::pushRequest(Request &request)
{
  return m_queue->push(request);
}

bool Dispatcher::requestHandler(const Request &request)
{
  switch (request.action) {
  case Dispatcher::Action::Connect:
    return doConnect(getShared(), request);
  case Dispatcher::Action::SendDescriptor:
    return doSendDescriptor(getShared(), request);
  case Dispatcher::Action::RequestSession:
    return doRequestSession(getShared(), request);
  case Dispatcher::Action::SetSession:
    return doSetSession(getShared(), request);
  case Dispatcher::Action::InitDatabase:
    return doInitDatabase(getShared(), request);
  case Dispatcher::Action::GetDevices:
    return doGetDevices(getShared(), request);
  case Dispatcher::Action::GetSessions:
    return doGetSessions(getShared(), request);
  case Dispatcher::Action::RemoveSession:
    return doRemoveSession(getShared(), request);
  case Dispatcher::Action::AddDevice:
    return doAddDevice(getShared(), request);
  case Dispatcher::Action::RemoveDevice:
    return doRemoveDevice(getShared(), request);
  case Dispatcher::Action::ConnectDevice:
    return doConnectDevice(getShared(), request);
  case Dispatcher::Action::DisconnectDevice:
    return doDisconnectDevice(getShared(), request);
  case Dispatcher::Action::StartCollecting:
    return doStartCollecting(getShared(), request);
  case Dispatcher::Action::StopCollecting:
    return doStopCollecting(getShared(), request);
  case Dispatcher::Action::QuitCollector:
    return doQuitCollector(getShared(), request);
  case Dispatcher::Action::CollectorStatus:
    return doCollectorStatus(getShared(), request);
  case Dispatcher::Action::DeviceList:
    return doDeviceList(getShared(), request);
  case Dispatcher::Action::SessionList:
    return doSessionList(getShared(), request);
  case Dispatcher::Action::Quit:
    return doQuit(getShared(), request);
  default:
    break;
  }

  return true;
}

static bool doConnect(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &)
{
  Dispatcher::Request rq;

  if (ControlApp()->getConnection()->connect() < 0) {
    std::cout << "Connection to collector failed" << std::endl;
    rq.action = Dispatcher::Action::Quit;
  } else {
    rq.action = Dispatcher::Action::SendDescriptor;
  }

  return mgr->pushRequest(rq);
}

static bool doSendDescriptor(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &)
{
  tkm::msg::control::Descriptor descriptor;
  descriptor.set_pid(getpid());

  if (!sendControlDescriptor(ControlApp()->getConnection()->getFD(), descriptor)) {
    logError() << "Failed to send descriptor";
    Dispatcher::Request nrq{.action = Dispatcher::Action::Quit};
    return mgr->pushRequest(nrq);
  }
  logDebug() << "Sent control descriptor";

  Dispatcher::Request nrq{.action = Dispatcher::Action::RequestSession};
  return mgr->pushRequest(nrq);
}

static bool doRequestSession(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;
  tkm::msg::control::SessionInfo sessionInfo;

  sessionInfo.set_id("Collector");

  requestMessage.set_id("RequestSession");
  requestMessage.set_type(msg::control::Request_Type_RequestSession);
  requestMessage.mutable_data()->PackFrom(sessionInfo);

  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request session: " << sessionInfo.id();
  return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static bool doSetSession(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  const auto &sessionInfo = std::any_cast<tkm::msg::control::SessionInfo>(rq.bulkData);

  logDebug() << "Server accepted: " << sessionInfo.id();
  ControlApp()->setSession(sessionInfo.id());

  return ControlApp()->getCommand()->trigger();
}

static bool doQuit(const std::shared_ptr<Dispatcher>, const Dispatcher::Request &)
{
  std::cout << std::flush;
  exit(EXIT_SUCCESS);
}

static bool doInitDatabase(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;

  requestMessage.set_id("InitDatabase");
  requestMessage.set_type(tkm::msg::control::Request_Type_InitDatabase);
  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      requestMessage.set_forced(tkm::msg::control::Request_Forced_Enforced);
    }
  }
  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request init database";
  return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static bool doGetDevices(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;

  requestMessage.set_id("GetDevices");
  requestMessage.set_type(tkm::msg::control::Request_Type_GetDevices);
  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request get devices";
  return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static bool doAddDevice(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;
  tkm::msg::control::DeviceData deviceData;

  requestMessage.set_id("AddDevice");
  requestMessage.set_type(tkm::msg::control::Request_Type_AddDevice);
  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      requestMessage.set_forced(tkm::msg::control::Request_Forced_Enforced);
    }
  }

  deviceData.set_state(tkm::msg::control::DeviceData_State_Unknown);
  deviceData.set_name(rq.args.at(Defaults::Arg::DeviceName));
  deviceData.set_address(rq.args.at(Defaults::Arg::DeviceAddress));
  deviceData.set_port(std::stoi(rq.args.at(Defaults::Arg::DevicePort)));
  deviceData.set_hash(hashForDevice(deviceData));

  requestMessage.mutable_data()->PackFrom(deviceData);

  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request add device for: " << rq.args.at(Defaults::Arg::DeviceName)
             << " with hash: " << deviceData.hash();
  return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static bool doRemoveDevice(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;
  tkm::msg::control::DeviceData deviceData;

  requestMessage.set_id("RemoveDevice");
  requestMessage.set_type(tkm::msg::control::Request_Type_RemoveDevice);
  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      requestMessage.set_forced(tkm::msg::control::Request_Forced_Enforced);
    }
  }

  deviceData.set_hash(rq.args.at(Defaults::Arg::DeviceHash));
  requestMessage.mutable_data()->PackFrom(deviceData);

  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request remove device for: " << rq.args.at(Defaults::Arg::DeviceHash);
  return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static bool doRemoveSession(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;
  tkm::msg::control::SessionData sessionData;

  requestMessage.set_id("RemoveSession");
  requestMessage.set_type(tkm::msg::control::Request_Type_RemoveSession);
  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      requestMessage.set_forced(tkm::msg::control::Request_Forced_Enforced);
    }
  }

  sessionData.set_hash(rq.args.at(Defaults::Arg::SessionHash));
  requestMessage.mutable_data()->PackFrom(sessionData);

  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request remove session for: " << rq.args.at(Defaults::Arg::SessionHash);
  return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static bool doConnectDevice(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;
  tkm::msg::control::DeviceData deviceData;

  requestMessage.set_id("ConnectDevice");
  requestMessage.set_type(tkm::msg::control::Request_Type_ConnectDevice);
  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      requestMessage.set_forced(tkm::msg::control::Request_Forced_Enforced);
    }
  }

  deviceData.set_hash(rq.args.at(Defaults::Arg::DeviceHash));
  requestMessage.mutable_data()->PackFrom(deviceData);

  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request connect device for: " << rq.args.at(Defaults::Arg::DeviceHash);
  return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static bool doDisconnectDevice(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;
  tkm::msg::control::DeviceData deviceData;

  requestMessage.set_id("DisconnectDevice");
  requestMessage.set_type(tkm::msg::control::Request_Type_DisconnectDevice);
  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      requestMessage.set_forced(tkm::msg::control::Request_Forced_Enforced);
    }
  }

  deviceData.set_hash(rq.args.at(Defaults::Arg::DeviceHash));
  requestMessage.mutable_data()->PackFrom(deviceData);

  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request disconnect device for: " << rq.args.at(Defaults::Arg::DeviceHash);
  return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static bool doStartCollecting(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;
  tkm::msg::control::DeviceData deviceData;

  requestMessage.set_id("StartCollecting");
  requestMessage.set_type(tkm::msg::control::Request_Type_StartCollecting);
  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      requestMessage.set_forced(tkm::msg::control::Request_Forced_Enforced);
    }
  }

  deviceData.set_hash(rq.args.at(Defaults::Arg::DeviceHash));
  requestMessage.mutable_data()->PackFrom(deviceData);

  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request start collecting device for: " << rq.args.at(Defaults::Arg::DeviceHash);
  return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static bool doStopCollecting(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;
  tkm::msg::control::DeviceData deviceData;

  requestMessage.set_id("StartCollecting");
  requestMessage.set_type(tkm::msg::control::Request_Type_StopCollecting);
  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      requestMessage.set_forced(tkm::msg::control::Request_Forced_Enforced);
    }
  }

  deviceData.set_hash(rq.args.at(Defaults::Arg::DeviceHash));
  requestMessage.mutable_data()->PackFrom(deviceData);

  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request stop collecting device for: " << rq.args.at(Defaults::Arg::DeviceHash);
  return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static bool doGetSessions(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;
  tkm::msg::control::DeviceData deviceData;

  requestMessage.set_id("GetSessions");
  requestMessage.set_type(tkm::msg::control::Request_Type_GetSessions);
  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      requestMessage.set_forced(tkm::msg::control::Request_Forced_Enforced);
    }
  }

  if (rq.args.count(Defaults::Arg::DeviceHash)) {
    deviceData.set_hash(rq.args.at(Defaults::Arg::DeviceHash));
  }
  requestMessage.mutable_data()->PackFrom(deviceData);

  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request get sessions";
  return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static bool doQuitCollector(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::control::Request requestMessage;

  requestMessage.set_id("QuitCollector");
  requestMessage.set_type(tkm::msg::control::Request_Type_QuitCollector);
  if (rq.args.count(Defaults::Arg::Forced)) {
    if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
      requestMessage.set_forced(tkm::msg::control::Request_Forced_Enforced);
    }
  }
  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

  logDebug() << "Request collector to quit";
  if (ControlApp()->getConnection()->writeEnvelope(requestEnvelope)) {
    std::cout << "Requested" << std::endl;
  } else {
    std::cout << "Requested failed" << std::endl;
  }

  // We always enqueue the quit request after status
  Dispatcher::Request nrq{.action = Dispatcher::Action::Quit};
  return mgr->pushRequest(nrq);
}

static bool doCollectorStatus(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  const auto &requestStatus = std::any_cast<tkm::msg::control::Status>(rq.bulkData);
  const auto &statusText = (requestStatus.what() == tkm::msg::control::Status_What_OK)
                               ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                               : tkmDefaults.valFor(Defaults::Val::StatusError);

  logDebug() << "Collector status(" << requestStatus.request_id() << "): " << statusText
             << " Reason: " << requestStatus.reason();

  if (requestStatus.request_id() == "RequestSession") {
    return true;
  }

  std::cout << "--------------------------------------------------" << std::endl;
  std::cout << "Status: " << statusText << " Reason: " << requestStatus.reason() << std::endl;
  std::cout << "--------------------------------------------------" << std::endl;

  // We always enqueue the quit request after status
  Dispatcher::Request nrq{.action = Dispatcher::Action::Quit};
  return mgr->pushRequest(nrq);
}

static bool doDeviceList(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  std::cout << "--------------------------------------------------" << std::endl;

  const auto &deviceList = std::any_cast<tkm::msg::control::DeviceList>(rq.bulkData);
  for (int i = 0; i < deviceList.device_size(); i++) {
    const tkm::msg::control::DeviceData &deviceData = deviceList.device(i);
    std::cout << "Id\t: " << deviceData.hash() << std::endl;
    std::cout << "Name\t: " << deviceData.name() << std::endl;
    std::cout << "Address\t: " << deviceData.address() << std::endl;
    std::cout << "Port\t: " << deviceData.port() << std::endl;
    std::cout << "State\t: ";
    switch (deviceData.state()) {
    case tkm::msg::control::DeviceData_State_Loaded:
      std::cout << "Loaded" << std::endl;
      break;
    case tkm::msg::control::DeviceData_State_Connected:
      std::cout << "Connected" << std::endl;
      break;
    case tkm::msg::control::DeviceData_State_Disconnected:
      std::cout << "Disconnected" << std::endl;
      break;
    case tkm::msg::control::DeviceData_State_Reconnecting:
      std::cout << "Reconnecting" << std::endl;
      break;
    case tkm::msg::control::DeviceData_State_Collecting:
      std::cout << "Collecting" << std::endl;
      break;
    case tkm::msg::control::DeviceData_State_Idle:
      std::cout << "Idle" << std::endl;
      break;
    case tkm::msg::control::DeviceData_State_Unknown:
    default:
      std::cout << "Unknown" << std::endl;
      break;
    }
    if (i < deviceList.device_size() - 1) {
      std::cout << std::endl;
    }
  }

  return true;
}

static bool doSessionList(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  std::cout << "--------------------------------------------------" << std::endl;

  const auto &sessionList = std::any_cast<tkm::msg::control::SessionList>(rq.bulkData);
  for (int i = 0; i < sessionList.session_size(); i++) {
    const tkm::msg::control::SessionData &sessionData = sessionList.session(i);
    std::cout << "Id\t: " << sessionData.hash() << std::endl;
    std::cout << "Name\t: " << sessionData.name() << std::endl;
    std::cout << "Started\t: " << sessionData.started() << std::endl;
    std::cout << "Ended\t: " << sessionData.ended() << std::endl;
    std::cout << "State\t: ";
    switch (sessionData.state()) {
    case tkm::msg::control::SessionData_State_Progress:
      std::cout << "Progress" << std::endl;
      break;
    case tkm::msg::control::SessionData_State_Complete:
      std::cout << "Complete" << std::endl;
      break;
    case tkm::msg::control::SessionData_State_Unknown:
    default:
      std::cout << "Unknown" << std::endl;
      break;
    }
    if (i < sessionList.session_size() - 1) {
      std::cout << std::endl;
    }
  }

  return true;
}

} // namespace tkm::control

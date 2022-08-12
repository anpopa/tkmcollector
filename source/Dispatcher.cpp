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

#include <taskmonitor/taskmonitor.h>
#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "Dispatcher.h"
#include "Helpers.h"

#include "../bswinfra/source/Timer.h"

namespace tkm::collector
{

static bool doInitDatabase(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doQuitCollector(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
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
static bool doQuit(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);
static bool doSendStatus(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq);

void Dispatcher::enableEvents()
{
  CollectorApp()->addEventSource(m_queue);
}

bool Dispatcher::pushRequest(Request &rq)
{
  return m_queue->push(rq);
}

bool Dispatcher::requestHandler(const Request &rq)
{
  switch (rq.action) {
  case Dispatcher::Action::InitDatabase:
    return doInitDatabase(getShared(), rq);
  case Dispatcher::Action::QuitCollector:
    return doQuitCollector(getShared(), rq);
  case Dispatcher::Action::GetDevices:
    return doGetDevices(getShared(), rq);
  case Dispatcher::Action::GetSessions:
    return doGetSessions(getShared(), rq);
  case Dispatcher::Action::RemoveSession:
    return doRemoveSession(getShared(), rq);
  case Dispatcher::Action::AddDevice:
    return doAddDevice(getShared(), rq);
  case Dispatcher::Action::RemoveDevice:
    return doRemoveDevice(getShared(), rq);
  case Dispatcher::Action::ConnectDevice:
    return doConnectDevice(getShared(), rq);
  case Dispatcher::Action::DisconnectDevice:
    return doDisconnectDevice(getShared(), rq);
  case Dispatcher::Action::StartCollecting:
    return doStartCollecting(getShared(), rq);
  case Dispatcher::Action::StopCollecting:
    return doStopCollecting(getShared(), rq);
  case Dispatcher::Action::SendStatus:
    return doSendStatus(getShared(), rq);
  case Dispatcher::Action::Quit:
    return doQuit(getShared(), rq);
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

static bool doInitDatabase(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  IDatabase::Request dbrq{
      .client = rq.client, .action = IDatabase::Action::InitDatabase, .args = rq.args};
  return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static bool doQuitCollector(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  // TODO: Do some cleanup like closing all client connections
  Dispatcher::Request quitRq{.action = Dispatcher::Action::Quit};
  return mgr->pushRequest(quitRq);
}

static bool doGetDevices(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  IDatabase::Request dbrq{
      .client = rq.client, .action = IDatabase::Action::GetDevices, .args = rq.args};
  return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static bool doRemoveSession(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  IDatabase::Request dbrq{.client = rq.client,
                          .action = IDatabase::Action::RemSession,
                          .args = rq.args,
                          .bulkData = rq.bulkData};
  return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static bool doAddDevice(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  IDatabase::Request dbrq{.client = rq.client,
                          .action = IDatabase::Action::AddDevice,
                          .args = rq.args,
                          .bulkData = rq.bulkData};
  return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static bool doRemoveDevice(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  IDatabase::Request dbrq{.client = rq.client,
                          .action = IDatabase::Action::RemoveDevice,
                          .args = rq.args,
                          .bulkData = rq.bulkData};
  return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static bool doConnectDevice(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  const auto &deviceData = std::any_cast<tkm::msg::control::DeviceData>(rq.bulkData);
  std::shared_ptr<MonitorDevice> device =
      CollectorApp()->getDeviceManager()->getDevice(deviceData.hash());

  if (device == nullptr) {
    Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};

    logDebug() << "No device entry in manager for " << deviceData.hash();
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason, "No such device");

    return CollectorApp()->getDispatcher()->pushRequest(mrq);
  }

  IDevice::Request drq{.client = rq.client,
                       .action = IDevice::Action::Connect,
                       .args = rq.args,
                       .bulkData = rq.bulkData};

  return device->pushRequest(drq);
}

static bool doDisconnectDevice(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  const auto &deviceData = std::any_cast<tkm::msg::control::DeviceData>(rq.bulkData);
  std::shared_ptr<MonitorDevice> device =
      CollectorApp()->getDeviceManager()->getDevice(deviceData.hash());

  if (device == nullptr) {
    Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};

    logDebug() << "No device entry in manager for " << deviceData.hash();
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason, "No such device");

    return CollectorApp()->getDispatcher()->pushRequest(mrq);
  }

  IDevice::Request drq{.client = rq.client,
                       .action = IDevice::Action::Disconnect,
                       .args = rq.args,
                       .bulkData = rq.bulkData};

  return device->pushRequest(drq);
}

static bool doStartCollecting(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  const auto &deviceData = std::any_cast<tkm::msg::control::DeviceData>(rq.bulkData);
  std::shared_ptr<MonitorDevice> device =
      CollectorApp()->getDeviceManager()->getDevice(deviceData.hash());

  if (device == nullptr) {
    Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};

    logDebug() << "No device entry in manager for " << deviceData.hash();
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason, "No such device");

    return CollectorApp()->getDispatcher()->pushRequest(mrq);
  }

  IDevice::Request drq{.client = rq.client,
                       .action = IDevice::Action::StartCollecting,
                       .args = rq.args,
                       .bulkData = rq.bulkData};

  return device->pushRequest(drq);
}

static bool doStopCollecting(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  const auto &deviceData = std::any_cast<tkm::msg::control::DeviceData>(rq.bulkData);
  std::shared_ptr<MonitorDevice> device =
      CollectorApp()->getDeviceManager()->getDevice(deviceData.hash());

  if (device == nullptr) {
    Dispatcher::Request mrq{.client = rq.client, .action = Dispatcher::Action::SendStatus};

    logDebug() << "No device entry in manager for " << deviceData.hash();
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
    mrq.args.emplace(Defaults::Arg::Reason, "No such device");

    return CollectorApp()->getDispatcher()->pushRequest(mrq);
  }

  IDevice::Request drq{.client = rq.client,
                       .action = IDevice::Action::StopCollecting,
                       .args = rq.args,
                       .bulkData = rq.bulkData};

  return device->pushRequest(drq);
}

static bool doGetSessions(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  IDatabase::Request dbrq{.client = rq.client,
                          .action = IDatabase::Action::GetSessions,
                          .args = rq.args,
                          .bulkData = rq.bulkData};
  return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static bool doQuit(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  exit(EXIT_SUCCESS);
}

static bool doSendStatus(const std::shared_ptr<Dispatcher> mgr, const Dispatcher::Request &rq)
{
  if (rq.client == nullptr) {
    logDebug() << "No client set for send status";
    return true;
  }

  tkm::msg::Envelope envelope;
  tkm::msg::control::Message message;
  tkm::msg::control::Status status;

  if (rq.args.count(Defaults::Arg::RequestId)) {
    status.set_request_id(rq.args.at(Defaults::Arg::RequestId));
  }

  if (rq.args.count(Defaults::Arg::Status)) {
    if (rq.args.at(Defaults::Arg::Status) == tkmDefaults.valFor(Defaults::Val::StatusOkay)) {
      status.set_what(tkm::msg::control::Status_What_OK);
    } else if (rq.args.at(Defaults::Arg::Status) == tkmDefaults.valFor(Defaults::Val::StatusBusy)) {
      status.set_what(tkm::msg::control::Status_What_Busy);
    } else {
      status.set_what(tkm::msg::control::Status_What_Error);
    }
  }

  if (rq.args.count(Defaults::Arg::Reason)) {
    status.set_reason(rq.args.at(Defaults::Arg::Reason));
  }

  // As response to client registration request we ask client to send descriptor
  message.set_type(tkm::msg::control::Message_Type_Status);
  message.mutable_data()->PackFrom(status);
  envelope.mutable_mesg()->PackFrom(message);

  envelope.set_target(tkm::msg::Envelope_Recipient_Any);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

  logDebug() << "Send status " << rq.args.at(Defaults::Arg::Status) << " to " << rq.client->getFD();
  return rq.client->writeEnvelope(envelope);
}

} // namespace tkm::collector

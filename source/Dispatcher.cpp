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
#include "Defaults.h"
#include "Dispatcher.h"
#include "Helpers.h"

#include "../bswinfra/source/Timer.h"

#include "Collector.pb.h"

using std::shared_ptr;
using std::string;

namespace tkm::collector
{

static auto doInitDatabase(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doQuitCollector(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doGetDevices(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doGetSessions(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doAddDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doRemoveDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doConnectDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doDisconnectDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doStartCollecting(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doStopCollecting(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doQuit(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doSendStatus(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;

void Dispatcher::enableEvents()
{
    CollectorApp()->addEventSource(m_queue);
}

auto Dispatcher::pushRequest(Request &rq) -> bool
{
    return m_queue->push(rq);
}

auto Dispatcher::requestHandler(const Request &rq) -> bool
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

static auto doInitDatabase(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {
        .client = rq.client, .action = IDatabase::Action::InitDatabase, .args = rq.args};
    return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static auto doQuitCollector(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    // TODO: Do some cleanup like closing all client connections
    Dispatcher::Request quitRq {.action = Dispatcher::Action::Quit};
    return mgr->pushRequest(quitRq);
}

static auto doGetDevices(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {
        .client = rq.client, .action = IDatabase::Action::GetDevices, .args = rq.args};
    return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static auto doAddDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::AddDevice,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static auto doRemoveDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::RemoveDevice,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static auto doConnectDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    const auto &deviceData = std::any_cast<tkm::msg::collector::DeviceData>(rq.bulkData);
    std::shared_ptr<MonitorDevice> device
        = CollectorApp()->getDeviceManager()->getDevice(deviceData.hash());

    if (device == nullptr) {
        Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

        logDebug() << "No device entry in manager for " << deviceData.hash();
        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "No such device");

        return CollectorApp()->getDispatcher()->pushRequest(mrq);
    }

    IDevice::Request drq {.client = rq.client,
                          .action = IDevice::Action::Connect,
                          .args = rq.args,
                          .bulkData = rq.bulkData};

    return device->pushRequest(drq);
}

static auto doDisconnectDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    const auto &deviceData = std::any_cast<tkm::msg::collector::DeviceData>(rq.bulkData);
    std::shared_ptr<MonitorDevice> device
        = CollectorApp()->getDeviceManager()->getDevice(deviceData.hash());

    if (device == nullptr) {
        Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

        logDebug() << "No device entry in manager for " << deviceData.hash();
        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "No such device");

        return CollectorApp()->getDispatcher()->pushRequest(mrq);
    }

    IDevice::Request drq {.client = rq.client,
                          .action = IDevice::Action::Disconnect,
                          .args = rq.args,
                          .bulkData = rq.bulkData};

    return device->pushRequest(drq);
}

static auto doStartCollecting(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    const auto &deviceData = std::any_cast<tkm::msg::collector::DeviceData>(rq.bulkData);
    std::shared_ptr<MonitorDevice> device
        = CollectorApp()->getDeviceManager()->getDevice(deviceData.hash());

    if (device == nullptr) {
        Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

        logDebug() << "No device entry in manager for " << deviceData.hash();
        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "No such device");

        return CollectorApp()->getDispatcher()->pushRequest(mrq);
    }

    IDevice::Request drq {.client = rq.client,
                          .action = IDevice::Action::StartCollecting,
                          .args = rq.args,
                          .bulkData = rq.bulkData};

    return device->pushRequest(drq);
}

static auto doStopCollecting(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    const auto &deviceData = std::any_cast<tkm::msg::collector::DeviceData>(rq.bulkData);
    std::shared_ptr<MonitorDevice> device
        = CollectorApp()->getDeviceManager()->getDevice(deviceData.hash());

    if (device == nullptr) {
        Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

        logDebug() << "No device entry in manager for " << deviceData.hash();
        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "No such device");

        return CollectorApp()->getDispatcher()->pushRequest(mrq);
    }

    IDevice::Request drq {.client = rq.client,
                          .action = IDevice::Action::StopCollecting,
                          .args = rq.args,
                          .bulkData = rq.bulkData};

    return device->pushRequest(drq);
}

static auto doGetSessions(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::GetSessions,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static auto doQuit(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    exit(EXIT_SUCCESS);
}

static auto doSendStatus(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    if (rq.client == nullptr) {
        logDebug() << "No client set for send status";
        return true;
    }

    tkm::msg::Envelope envelope;
    tkm::msg::collector::Message message;
    tkm::msg::collector::Status status;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        status.set_requestid(rq.args.at(Defaults::Arg::RequestId));
    }

    if (rq.args.count(Defaults::Arg::Status)) {
        if (rq.args.at(Defaults::Arg::Status) == tkmDefaults.valFor(Defaults::Val::StatusOkay)) {
            status.set_what(tkm::msg::collector::Status_What_OK);
        } else if (rq.args.at(Defaults::Arg::Status)
                   == tkmDefaults.valFor(Defaults::Val::StatusBusy)) {
            status.set_what(tkm::msg::collector::Status_What_Busy);
        } else {
            status.set_what(tkm::msg::collector::Status_What_Error);
        }
    }

    if (rq.args.count(Defaults::Arg::Reason)) {
        status.set_reason(rq.args.at(Defaults::Arg::Reason));
    }

    // As response to client registration request we ask client to send descriptor
    message.set_type(tkm::msg::collector::Message_Type_Status);
    message.mutable_data()->PackFrom(status);
    envelope.mutable_mesg()->PackFrom(message);

    envelope.set_target(tkm::msg::Envelope_Recipient_Any);
    envelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    logDebug() << "Send status " << rq.args.at(Defaults::Arg::Status) << " to "
               << rq.client->getFD();
    return rq.client->writeEnvelope(envelope);
}

} // namespace tkm::collector

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
#include "Server.pb.h"

using std::shared_ptr;
using std::string;

namespace tkm::collector
{
static auto doConnect(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;
static auto doDisconnect(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;
static auto doSendDescriptor(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;
static auto doRequestSession(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;
static auto doSetSession(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;
static auto doStartCollecting(const shared_ptr<MonitorDevice> &mgr,
                              const MonitorDevice::Request &rq) -> bool;
static auto doStopCollecting(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;
static auto doStartStream(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;
static auto doStopStream(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;
static auto doProcessData(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;
static auto doStatus(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;

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

auto MonitorDevice::pushRequest(Request &request) -> bool
{
    return m_queue->push(request);
}

void MonitorDevice::updateState(tkm::msg::collector::DeviceData_State state)
{
    m_deviceData.set_state(state);

    if (state == tkm::msg::collector::DeviceData_State_Disconnected) {
        if (m_sessionData.hash().length() > 0) {
            IDatabase::Request dbrq {.action = IDatabase::Action::EndSession};
            dbrq.args.emplace(Defaults::Arg::SessionHash, m_sessionData.hash());
            CollectorApp()->getDatabase()->pushRequest(dbrq);
        }
    }
}

auto MonitorDevice::requestHandler(const Request &request) -> bool
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

static auto doConnect(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    if (!mgr->createConnection()) {
        logError() << "Connection to device failed";
        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Failed to create connection object");
        return CollectorApp()->getDispatcher()->pushRequest(mrq);
    }
    mgr->enableConnection();

    if (mgr->getConnection()->connect() < 0) {
        logError() << "Connection to device failed";

        // Remove connection object
        mgr->getConnection() = nullptr;

        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Connection Failed");
        return CollectorApp()->getDispatcher()->pushRequest(mrq);
    }

    MonitorDevice::Request nrq = {.client = rq.client,
                                  .action = MonitorDevice::Action::SendDescriptor,
                                  .args = rq.args,
                                  .bulkData = rq.bulkData};

    return mgr->pushRequest(nrq);
}

static auto doSendDescriptor(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};
    tkm::msg::client::Descriptor descriptor;
    descriptor.set_id("Collector");

    if (!sendClientDescriptor(mgr->getConnection()->getFD(), descriptor)) {
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
    mgr->updateState(tkm::msg::collector::DeviceData_State_Connected);

    logDebug() << "Client connected";
    return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doRequestSession(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool
{
    tkm::msg::Envelope envelope;
    tkm::msg::client::Request request;

    request.set_id("CreateSession");
    request.set_type(tkm::msg::client::Request::Type::Request_Type_CreateSession);

    envelope.mutable_mesg()->PackFrom(request);
    envelope.set_target(tkm::msg::Envelope_Recipient_Server);
    envelope.set_origin(tkm::msg::Envelope_Recipient_Client);

    logDebug() << "Request session to server";
    return mgr->getConnection()->writeEnvelope(envelope);
}

static auto doSetSession(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool
{
    const auto &sessionInfo = std::any_cast<tkm::msg::server::SessionInfo>(rq.bulkData);

    // Update our session data
    logDebug() << "Session created: " << sessionInfo.id();
    mgr->getSessionData().set_hash(sessionInfo.id());
    mgr->updateState(tkm::msg::collector::DeviceData_State_SessionSet);

    // Create session
    IDatabase::Request dbrq {.action = IDatabase::Action::AddSession};
    dbrq.args.emplace(Defaults::Arg::DeviceHash, mgr->getDeviceData().hash());
    dbrq.args.emplace(Defaults::Arg::SessionHash, mgr->getSessionData().hash());
    CollectorApp()->getDatabase()->pushRequest(dbrq);

    // Start data stream
    MonitorDevice::Request nrq = {.action = MonitorDevice::Action::StartStream};
    return mgr->pushRequest(nrq);
}

static auto doDisconnect(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    if (mgr->getDeviceData().state() != tkm::msg::collector::DeviceData_State_Disconnected) {
        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
        mrq.args.emplace(Defaults::Arg::Reason, "Device disconnected");

        // Delete the current connection
        mgr->deleteConnection();
    } else {
        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Device not connected");
    }

    logDebug() << mrq.args.at(Defaults::Arg::Reason);
    return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doStartStream(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &)
    -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::client::Request requestMessage;
    tkm::msg::client::StreamState streamState;

    streamState.set_state(true);
    requestMessage.set_id("StartStream");
    requestMessage.set_type(tkm::msg::client::Request_Type_StreamState);
    requestMessage.mutable_data()->PackFrom(streamState);

    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Server);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Client);

    logDebug() << "Request start stream";
    mgr->getDeviceData().set_state(tkm::msg::collector::DeviceData_State_Collecting);
    return mgr->getConnection()->writeEnvelope(requestEnvelope);
}

static auto doStopStream(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &)
    -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::client::Request requestMessage;
    tkm::msg::client::StreamState streamState;

    streamState.set_state(false);
    requestMessage.set_id("StopStream");
    requestMessage.set_type(tkm::msg::client::Request_Type_StreamState);
    requestMessage.mutable_data()->PackFrom(streamState);

    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Server);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Client);

    logDebug() << "Request stop stream";
    mgr->getDeviceData().set_state(tkm::msg::collector::DeviceData_State_Idle);
    return mgr->getConnection()->writeEnvelope(requestEnvelope);
}

static auto doStartCollecting(const shared_ptr<MonitorDevice> &mgr,
                              const MonitorDevice::Request &rq) -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    if ((mgr->getDeviceData().state() == tkm::msg::collector::DeviceData_State_Connected)
        || (mgr->getDeviceData().state() == tkm::msg::collector::DeviceData_State_Idle)) {
        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
        mrq.args.emplace(Defaults::Arg::Reason, "Collecting requested");

        MonitorDevice::Request nrq = {.action = MonitorDevice::Action::RequestSession};
        mgr->pushRequest(nrq);
    } else if (mgr->getDeviceData().state() == tkm::msg::collector::DeviceData_State_SessionSet) {
        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
        mrq.args.emplace(Defaults::Arg::Reason, "Collecting requested");

        MonitorDevice::Request nrq = {.action = MonitorDevice::Action::StartStream};
        mgr->pushRequest(nrq);
    } else {
        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Device not connected");
    }

    return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doStopCollecting(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }

    if (mgr->getDeviceData().state() == tkm::msg::collector::DeviceData_State_Collecting) {
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

static auto doProcessData(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool
{
    // Add entry to database
    IDatabase::Request dbrq {.action = IDatabase::Action::AddData, .bulkData = rq.bulkData};
    dbrq.args.emplace(Defaults::Arg::SessionHash, mgr->getSessionData().hash());
    return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

static auto doStatus(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq) -> bool
{
    const auto &serverStatus = std::any_cast<tkm::msg::server::Status>(rq.bulkData);
    std::string what;

    switch (serverStatus.what()) {
    case tkm::msg::server::Status_What_OK:
        what = tkmDefaults.valFor(Defaults::Val::StatusOkay);
        break;
    case tkm::msg::server::Status_What_Busy:
        what = tkmDefaults.valFor(Defaults::Val::StatusBusy);
        break;
    case tkm::msg::server::Status_What_Error:
    default:
        what = tkmDefaults.valFor(Defaults::Val::StatusError);
        break;
    }

    logDebug() << "Server status (" << serverStatus.requestid() << "): " << what
               << " Reason: " << serverStatus.reason();

    return true;
}

} // namespace tkm::collector
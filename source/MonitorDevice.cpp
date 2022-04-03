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
#include <json/json.h>
#include <ostream>
#include <string>
#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "Dispatcher.h"
#include "Helpers.h"
#include "MonitorDevice.h"

#include "Collector.pb.h"
#include "Server.pb.h"

using std::shared_ptr;
using std::string;

namespace tkm::collector
{

static auto splitString(const std::string &s, char delim) -> std::vector<std::string>;
static void printProcAcct(const tkm::msg::server::ProcAcct &acct, uint64_t ts);
static void printProcEvent(const tkm::msg::server::ProcEvent &event, uint64_t ts);
static void printSysProcStat(const tkm::msg::server::SysProcStat &sysProcStat, uint64_t ts);
static void printSysProcPressure(const tkm::msg::server::SysProcPressure &sysProcPressure,
                                 uint64_t ts);

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
static auto doStartStream(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;
static auto doProcessData(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool;

void MonitorDevice::enableEvents()
{
    CollectorApp()->addEventSource(m_queue);
}

auto MonitorDevice::pushRequest(Request &request) -> bool
{
    return m_queue->push(request);
}

void MonitorDevice::notifyConnection(tkm::msg::collector::DeviceData_State state)
{
    m_deviceData.set_state(state);
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
    case MonitorDevice::Action::StartStream:
        return doStartStream(getShared(), request);
    case MonitorDevice::Action::ProcessData:
        return doProcessData(getShared(), request);
    default:
        break;
    }

    logError() << "Unknown action request";
    return false;
}

static auto doConnect(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool
{
    if (mgr->getConnection()->connect() < 0) {
        Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

        if (rq.args.count(Defaults::Arg::RequestId)) {
            mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
        }

        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Connection Failed");

        logError() << "Connection to device failed";
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
    tkm::msg::client::Descriptor descriptor;
    descriptor.set_id("Collector");

    if (!sendClientDescriptor(mgr->getConnection()->getFD(), descriptor)) {
        Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

        if (rq.args.count(Defaults::Arg::RequestId)) {
            mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
        }

        mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
        mrq.args.emplace(Defaults::Arg::Reason, "Failed to send descriptor");

        logError() << "Failed to send descriptor";
        return CollectorApp()->getDispatcher()->pushRequest(mrq);
    }

    logDebug() << "Sent client descriptor";
    MonitorDevice::Request nrq {.action = MonitorDevice::Action::RequestSession};
    return mgr->pushRequest(nrq);
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

    logDebug() << "Server accepted: " << sessionInfo.id();
    mgr->getSessionData().set_hash(sessionInfo.id());

    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
    mrq.args.emplace(Defaults::Arg::Reason, "Session created");

    logError() << "Session created";
    mgr->notifyConnection(tkm::msg::collector::DeviceData_State_Connected);
    return CollectorApp()->getDispatcher()->pushRequest(mrq);
}

static auto doDisconnect(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool
{
    Dispatcher::Request mrq {.client = rq.client, .action = Dispatcher::Action::SendStatus};

    if (rq.args.count(Defaults::Arg::RequestId)) {
        mrq.args.emplace(Defaults::Arg::RequestId, rq.args.at(Defaults::Arg::RequestId));
    }
    mrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
    mrq.args.emplace(Defaults::Arg::Reason, "Device disconnected");

    mgr->getConnection()->disconnect();

    logError() << "Device disconnected";
    mgr->notifyConnection(tkm::msg::collector::DeviceData_State_Disconnected);
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
    return mgr->getConnection()->writeEnvelope(requestEnvelope);
}

static auto doProcessData(const shared_ptr<MonitorDevice> &mgr, const MonitorDevice::Request &rq)
    -> bool
{
    const auto &data = std::any_cast<tkm::msg::server::Data>(rq.bulkData);

    switch (data.what()) {
    case tkm::msg::server::Data_What_ProcAcct: {
        tkm::msg::server::ProcAcct procAcct;
        data.payload().UnpackTo(&procAcct);
        printProcAcct(procAcct, data.timestamp());
        break;
    }
    case tkm::msg::server::Data_What_ProcEvent: {
        tkm::msg::server::ProcEvent procEvent;
        data.payload().UnpackTo(&procEvent);
        printProcEvent(procEvent, data.timestamp());
        break;
    }
    case tkm::msg::server::Data_What_SysProcStat: {
        tkm::msg::server::SysProcStat sysProcStat;
        data.payload().UnpackTo(&sysProcStat);
        printSysProcStat(sysProcStat, data.timestamp());
        break;
    }
    case tkm::msg::server::Data_What_SysProcPressure: {
        tkm::msg::server::SysProcPressure sysProcPressure;
        data.payload().UnpackTo(&sysProcPressure);
        printSysProcPressure(sysProcPressure, data.timestamp());
        break;
    }
    default:
        break;
    }

    return true;
}

static void printProcAcct(const tkm::msg::server::ProcAcct &acct, uint64_t ts) { }

static void printProcEvent(const tkm::msg::server::ProcEvent &event, uint64_t ts) { }

static void printSysProcStat(const tkm::msg::server::SysProcStat &sysProcStat, uint64_t ts) { }

static void printSysProcPressure(const tkm::msg::server::SysProcPressure &sysProcPressure,
                                 uint64_t ts)
{
}

} // namespace tkm::collector
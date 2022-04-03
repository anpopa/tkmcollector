#include <unistd.h>

#include "Application.h"
#include "Dispatcher.h"
#include "Helpers.h"

#include "Collector.pb.h"

using std::shared_ptr;
using std::string;

namespace tkm::control
{

static auto doConnect(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doSendDescriptor(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doRequestSession(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doSetSession(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doQuit(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doInitDatabase(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
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
static auto doQuitCollector(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doCollectorStatus(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doDeviceList(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doSessionList(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;

void Dispatcher::enableEvents()
{
    ControlApp()->addEventSource(m_queue);
}

auto Dispatcher::pushRequest(Request &request) -> bool
{
    return m_queue->push(request);
}

auto Dispatcher::requestHandler(const Request &request) -> bool
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

static auto doConnect(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &) -> bool
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

static auto doSendDescriptor(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &) -> bool
{
    tkm::msg::collector::Descriptor descriptor;
    descriptor.set_pid(getpid());

    if (!sendControlDescriptor(ControlApp()->getConnection()->getFD(), descriptor)) {
        logError() << "Failed to send descriptor";
        Dispatcher::Request nrq {.action = Dispatcher::Action::Quit};
        return mgr->pushRequest(nrq);
    }
    logDebug() << "Sent control descriptor";

    Dispatcher::Request nrq {.action = Dispatcher::Action::RequestSession};
    return mgr->pushRequest(nrq);
}

static auto doRequestSession(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;
    tkm::msg::collector::SessionInfo sessionInfo;

    sessionInfo.set_id("Collector");

    requestMessage.set_id("RequestSession");
    requestMessage.set_type(msg::collector::Request_Type_RequestSession);
    requestMessage.mutable_data()->PackFrom(sessionInfo);

    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

    logDebug() << "Request session: " << sessionInfo.id();
    return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static auto doSetSession(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    const auto &sessionInfo = std::any_cast<tkm::msg::collector::SessionInfo>(rq.bulkData);

    logDebug() << "Server accepted: " << sessionInfo.id();
    ControlApp()->setSession(sessionInfo.id());

    return ControlApp()->getCommand()->trigger();
}

static auto doQuit(const shared_ptr<Dispatcher> &, const Dispatcher::Request &) -> bool
{
    std::cout << std::flush;
    exit(EXIT_SUCCESS);
}

static auto doInitDatabase(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    requestMessage.set_id("InitDatabase");
    requestMessage.set_type(tkm::msg::collector::Request_Type_InitDatabase);
    if (rq.args.count(Defaults::Arg::Forced)) {
        if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
            requestMessage.set_forced(tkm::msg::collector::Request_Forced_Enforced);
        }
    }
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

    logDebug() << "Request init database";
    return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static auto doGetDevices(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &) -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    requestMessage.set_id("GetDevices");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetDevices);
    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

    logDebug() << "Request get devices";
    return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static auto doAddDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;
    tkm::msg::collector::DeviceData deviceData;

    requestMessage.set_id("AddDevice");
    requestMessage.set_type(tkm::msg::collector::Request_Type_AddDevice);
    if (rq.args.count(Defaults::Arg::Forced)) {
        if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
            requestMessage.set_forced(tkm::msg::collector::Request_Forced_Enforced);
        }
    }

    deviceData.set_state(tkm::msg::collector::DeviceData_State_Unknown);
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

static auto doRemoveDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;
    tkm::msg::collector::DeviceData deviceData;

    requestMessage.set_id("ConnectDevice");
    requestMessage.set_type(tkm::msg::collector::Request_Type_RemoveDevice);
    if (rq.args.count(Defaults::Arg::Forced)) {
        if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
            requestMessage.set_forced(tkm::msg::collector::Request_Forced_Enforced);
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

static auto doConnectDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;
    tkm::msg::collector::DeviceData deviceData;

    requestMessage.set_id("ConnectDevice");
    requestMessage.set_type(tkm::msg::collector::Request_Type_ConnectDevice);
    if (rq.args.count(Defaults::Arg::Forced)) {
        if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
            requestMessage.set_forced(tkm::msg::collector::Request_Forced_Enforced);
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

static auto doDisconnectDevice(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;
    tkm::msg::collector::DeviceData deviceData;

    requestMessage.set_id("DisconnectDevice");
    requestMessage.set_type(tkm::msg::collector::Request_Type_DisconnectDevice);
    if (rq.args.count(Defaults::Arg::Forced)) {
        if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
            requestMessage.set_forced(tkm::msg::collector::Request_Forced_Enforced);
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

static auto doStartCollecting(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;
    tkm::msg::collector::DeviceData deviceData;

    requestMessage.set_id("StartCollecting");
    requestMessage.set_type(tkm::msg::collector::Request_Type_StartCollecting);
    if (rq.args.count(Defaults::Arg::Forced)) {
        if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
            requestMessage.set_forced(tkm::msg::collector::Request_Forced_Enforced);
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

static auto doStopCollecting(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;
    tkm::msg::collector::DeviceData deviceData;

    requestMessage.set_id("StartCollecting");
    requestMessage.set_type(tkm::msg::collector::Request_Type_StopCollecting);
    if (rq.args.count(Defaults::Arg::Forced)) {
        if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
            requestMessage.set_forced(tkm::msg::collector::Request_Forced_Enforced);
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

static auto doGetSessions(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;
    tkm::msg::collector::DeviceData deviceData;

    requestMessage.set_id("GetSessions");
    requestMessage.set_type(tkm::msg::collector::Request_Type_GetSessions);
    if (rq.args.count(Defaults::Arg::Forced)) {
        if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
            requestMessage.set_forced(tkm::msg::collector::Request_Forced_Enforced);
        }
    }

    deviceData.set_hash(rq.args.at(Defaults::Arg::DeviceHash));
    requestMessage.mutable_data()->PackFrom(deviceData);

    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Collector);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Control);

    logDebug() << "Request get sessions for device: " << rq.args.at(Defaults::Arg::DeviceHash);
    return ControlApp()->getConnection()->writeEnvelope(requestEnvelope);
}

static auto doQuitCollector(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Request requestMessage;

    requestMessage.set_id("QuitCollector");
    requestMessage.set_type(tkm::msg::collector::Request_Type_QuitCollector);
    if (rq.args.count(Defaults::Arg::Forced)) {
        if (rq.args.at(Defaults::Arg::Forced) == tkmDefaults.valFor(Defaults::Val::True)) {
            requestMessage.set_forced(tkm::msg::collector::Request_Forced_Enforced);
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
    Dispatcher::Request nrq {.action = Dispatcher::Action::Quit};
    return mgr->pushRequest(nrq);
}

static auto doCollectorStatus(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    const auto &requestStatus = std::any_cast<tkm::msg::collector::Status>(rq.bulkData);
    const auto &statusText = (requestStatus.what() == tkm::msg::collector::Status_What_OK)
                                 ? tkmDefaults.valFor(Defaults::Val::StatusOkay)
                                 : tkmDefaults.valFor(Defaults::Val::StatusError);

    logDebug() << "Collector status(" << requestStatus.requestid() << "): " << statusText
               << " Reason: " << requestStatus.reason();

    if (requestStatus.requestid() == "RequestSession") {
        return true;
    }

    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Status: " << statusText << " Reason: " << requestStatus.reason() << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    // We always enqueue the quit request after status
    Dispatcher::Request nrq {.action = Dispatcher::Action::Quit};
    return mgr->pushRequest(nrq);
}

static auto doDeviceList(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    std::cout << "--------------------------------------------------" << std::endl;

    const auto &deviceList = std::any_cast<tkm::msg::collector::DeviceList>(rq.bulkData);
    for (int i = 0; i < deviceList.device_size(); i++) {
        const tkm::msg::collector::DeviceData &deviceData = deviceList.device(i);
        std::cout << "Id\t: " << deviceData.hash() << std::endl;
        std::cout << "Name\t: " << deviceData.name() << std::endl;
        std::cout << "Address\t: " << deviceData.address() << std::endl;
        std::cout << "Port\t: " << deviceData.port() << std::endl;
        std::cout << "State\t: ";
        switch (deviceData.state()) {
        case tkm::msg::collector::DeviceData_State_Connected:
            std::cout << "Connected" << std::endl;
            break;
        case tkm::msg::collector::DeviceData_State_Disconnected:
            std::cout << "Disconnected" << std::endl;
            break;
        case tkm::msg::collector::DeviceData_State_Reconnecting:
            std::cout << "Reconnecting" << std::endl;
            break;
        case tkm::msg::collector::DeviceData_State_Collecting:
            std::cout << "Collecting" << std::endl;
            break;
        case tkm::msg::collector::DeviceData_State_Idle:
            std::cout << "Idle" << std::endl;
            break;
        case tkm::msg::collector::DeviceData_State_Unknown:
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

static auto doSessionList(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    std::cout << "--------------------------------------------------" << std::endl;

    const auto &sessionList = std::any_cast<tkm::msg::collector::SessionList>(rq.bulkData);
    for (int i = 0; i < sessionList.session_size(); i++) {
        const tkm::msg::collector::SessionData &sessionData = sessionList.session(i);
        std::cout << "Id\t: " << sessionData.hash() << std::endl;
        std::cout << "Name\t: " << sessionData.name() << std::endl;
        std::cout << "Started\t: " << sessionData.started() << std::endl;
        std::cout << "Ended\t: " << sessionData.ended() << std::endl;
        std::cout << "State\t: ";
        switch (sessionData.state()) {
        case tkm::msg::collector::SessionData_State_Progress:
            std::cout << "Progress" << std::endl;
            break;
        case tkm::msg::collector::SessionData_State_Complete:
            std::cout << "Complete" << std::endl;
            break;
        case tkm::msg::collector::SessionData_State_Unknown:
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

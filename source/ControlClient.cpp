/*-
 * Copyright (c) 2020 Alin Popa
 * All rights reserved.
 */

/*
 * @author Alin Popa <alin.popa@fxdata.ro>
 */

#include <unistd.h>

#include "ControlClient.h"
#include "Application.h"
#include "Defaults.h"
#include "Helpers.h"

#include "Collector.pb.h"

using std::shared_ptr;
using std::string;

namespace tkm::collector
{

static auto doInitDatabase(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
    -> bool;
static auto doRequestSession(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
    -> bool;
static auto doTerminateCollector(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
    -> bool;
static auto doGetDevices(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
    -> bool;
static auto doAddDevice(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
    -> bool;
static auto doRemoveDevice(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
    -> bool;
static auto doConnectDevice(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq) -> bool;
static auto doDisconnectDevice(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
    -> bool;
static auto doStartDeviceSession(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
    -> bool;
static auto doStopDeviceSession(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
    -> bool;

ControlClient::ControlClient(int clientFd)
: IClient("ControlClient", clientFd)
{
    bswi::event::Pollable::lateSetup(
        [this]() {
            auto status = true;

            do {
                cds::msg::Envelope envelope;

                // Read next message
                auto readStatus = readEnvelope(envelope);
                if (readStatus == IAsyncEnvelope::Status::Again) {
                    return true;
                } else if (readStatus == IAsyncEnvelope::Status::Error) {
                    logDebug() << "ControlClient read error";
                    return false;
                } else if (readStatus == IAsyncEnvelope::Status::EndOfFile) {
                    logDebug() << "ControlClient read end of file";
                    return false;
                }

                // Check for valid origin
                if (envelope.origin() != tkm::msg::Envelope_Recipient_Control) {
                    continue;
                }

                tkm::msg::collector::Request rq;
                envelope.mesg().UnpackTo(&rq);

                switch (msg.type()) {
                case tkm::msg::collector::Request_Type_RequestSession:
                    status = doRequestSession(getShared(), rq);
                    break;
                case tkm::msg::collector::Request_Type_InitDatabase:
                    status = doInitDatabase(getShared(), rq);
                    break;
                case tkm::msg::collector::Request_Type_TerminateCollector:
                    status = doTerminateCollector(getShared(), rq);
                    break;
                case tkm::msg::collector::Request_Type_GetDevices:
                    status = doGetDevices(getShared(), rq);
                    break;
                case tkm::msg::collector::Request_Type_AddDevice:
                    status = doAddDevice(getShared(), rq);
                    break;
                case tkm::msg::collector::Request_Type_RemoveDevice:
                    status = doRemoveDevicegetShared(), rq);
                    break;
                case tkm::msg::collector::Request_Type_ConnectDevice:
                    status = doConnectDevice(getShared(), rq);
                    break;
                case tkm::msg::collector::Request_Type_DisconnectDevice:
                    status = doDisconnectDevice(getShared(), rq);
                    break;
                case tkm::msg::collector::Request_Type_StartDeviceSession:
                    status = doStartDeviceSession(getShared(), msg);
                    break;
                case tkm::msg::collector::Request_Type_StopDeviceSession:
                    status = doStopDeviceSession(getShared(), msg);
                    break;
                default:
                    status = false;
                    break;
                }
            } while (status);

            return status;
        },
        getFD(),
        bswi::event::IPollable::Events::Level,
        bswi::event::IEventSource::Priority::Normal);

    setFinalize([this]() { logDebug() << "Ended connection with client: " << getName(); });
}

void ControlClient::enableEvents()
{
    CollectorApp()->addEventSource(getShared());
}

ControlClient::~ControlClient()
{
    if (m_clientFd > 0) {
        ::close(m_clientFd);
        m_clientFd = -1;
    }
}

static auto doRequestSession(const shared_ptr<ControlClient> &client, tkm::msg::collector::Request &rq)
    -> bool
{
    tkm::msg::collector::SessionInfo sessionInfo;
    rq.data().UnpackTo(&sessionInfo);

    tkm::msg::Envelope requestEnvelope;
    tkm::msg::collector::Message requestMessage;

    requestMessage.set_type(tkm::msg::collector::Message_Type_SetSession);
    requestMessage.mutable_data()->PackFrom(sessionInfo);

    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Control);
    requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    auto status = client->writeEnvelope(requestEnvelope);

    Dispatcher::Request nrq {.client = client, .action = Dispatcher::Action::SendStatus};
    nrq.args.emplace(Defaults::Arg::RequestId, rq.id());
    if (!status) {
        nrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusError));
        nrq.args.emplace(Defaults::Arg::Reason, "Failed to set session");
    } else {
        nrq.args.emplace(Defaults::Arg::Status, tkmDefaults.valFor(Defaults::Val::StatusOkay));
        nrq.args.emplace(Defaults::Arg::Reason, "Session set");
    }

    return CollectorApp()->getDispatcher()->pushRequest(nrq);
}

static auto doInitDatabase(const shared_ptr<AdminClient> &client, cds::msg::admin::Request &msg)
    -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::InitDatabase};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());

    if (msg.forced() == cds::msg::admin::Request_Forced_Enforced) {
        rq.args.emplace(Defaults::Arg::Forced, cdsDefaults.valFor(Defaults::Val::True));
    }

    return CDSApp()->getDispatcher()->pushRequest(rq);
}

static auto doTerminateServer(const shared_ptr<AdminClient> &client, cds::msg::admin::Request &msg)
    -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::TerminateServer};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());

    if (msg.forced() == cds::msg::admin::Request_Forced_Enforced) {
        rq.args.emplace(Defaults::Arg::Forced, cdsDefaults.valFor(Defaults::Val::True));
    }

    return CDSApp()->getDispatcher()->pushRequest(rq);
}

static auto doGetUsers(const shared_ptr<AdminClient> &client, cds::msg::admin::Request &msg) -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::GetUsers};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());
    return CDSApp()->getDispatcher()->pushRequest(rq);
}

static auto doGetProjects(const shared_ptr<AdminClient> &client, cds::msg::admin::Request &msg)
    -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::GetProjects};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());
    return CDSApp()->getDispatcher()->pushRequest(rq);
}

static auto doGetWorkPackages(const shared_ptr<AdminClient> &client, cds::msg::admin::Request &msg)
    -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::GetWorkPackages};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());
    return CDSApp()->getDispatcher()->pushRequest(rq);
}

static auto doAddUser(const shared_ptr<AdminClient> &client, cds::msg::admin::Request &msg) -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::AddUser};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());

    if (msg.forced() == cds::msg::admin::Request_Forced_Enforced) {
        rq.args.emplace(Defaults::Arg::Forced, cdsDefaults.valFor(Defaults::Val::True));
    }

    cds::msg::server::UserData data;
    msg.data().UnpackTo(&data);
    rq.bulkData = std::make_any<cds::msg::server::UserData>(data);

    return CDSApp()->getDispatcher()->pushRequest(rq);
}

static auto doUpdateUser(const shared_ptr<AdminClient> &client, cds::msg::admin::Request &msg)
    -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::UpdateUser};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());

    if (msg.forced() == cds::msg::admin::Request_Forced_Enforced) {
        rq.args.emplace(Defaults::Arg::Forced, cdsDefaults.valFor(Defaults::Val::True));
    }

    cds::msg::server::UserData data;
    msg.data().UnpackTo(&data);
    rq.bulkData = std::make_any<cds::msg::server::UserData>(data);

    return CDSApp()->getDispatcher()->pushRequest(rq);
}

static auto doAddProject(const shared_ptr<AdminClient> &client, cds::msg::admin::Request &msg)
    -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::AddProject};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());

    cds::msg::server::ProjectData data;
    msg.data().UnpackTo(&data);
    rq.bulkData = std::make_any<cds::msg::server::ProjectData>(data);

    return CDSApp()->getDispatcher()->pushRequest(rq);
}

static auto doRemoveUser(const shared_ptr<AdminClient> &client, cds::msg::admin::Request &msg)
    -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::RemoveUser};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());

    cds::msg::server::UserData data;
    msg.data().UnpackTo(&data);
    rq.args.emplace(Defaults::Arg::Name, data.name());

    return CDSApp()->getDispatcher()->pushRequest(rq);
}

static auto doRemoveProject(const shared_ptr<AdminClient> &client, cds::msg::admin::Request &msg)
    -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::RemoveProject};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());

    cds::msg::server::ProjectData data;
    msg.data().UnpackTo(&data);
    rq.args.emplace(Defaults::Arg::Name, data.name());

    return CDSApp()->getDispatcher()->pushRequest(rq);
}

static auto doAddWorkPackage(const shared_ptr<AdminClient> &client, cds::msg::admin::Request &msg)
    -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::AddWorkPackage};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());

    cds::msg::server::WorkPackageData data;
    msg.data().UnpackTo(&data);
    rq.bulkData = std::make_any<cds::msg::server::WorkPackageData>(data);

    return CDSApp()->getDispatcher()->pushRequest(rq);
}

static auto doRemoveWorkPackage(const shared_ptr<AdminClient> &client,
                                cds::msg::admin::Request &msg) -> bool
{
    Dispatcher::Request rq {.client = client, .action = Dispatcher::Action::RemoveWorkPackage};
    rq.args.emplace(Defaults::Arg::RequestId, msg.id());

    cds::msg::server::WorkPackageData data;
    msg.data().UnpackTo(&data);
    rq.args.emplace(Defaults::Arg::Name, data.name());

    return CDSApp()->getDispatcher()->pushRequest(rq);
}

} // namespace cds::server

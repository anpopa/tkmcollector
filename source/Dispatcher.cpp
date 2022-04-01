/*-
 * Copyright (c) 2020 Alin Popa
 * All rights reserved.
 */

/*
 * @author Alin Popa <alin.popa@fxdata.ro>
 */

#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "Dispatcher.h"
#include "Helpers.h"

#include "../bswinfra/source/Timer.h"

#include "Server.pb.h"

using std::shared_ptr;
using std::string;

namespace cds::server
{

static auto doInitDatabase(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doTerminateServer(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doAuthUser(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doAuthStatus(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doGetUsers(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doGetProjects(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doGetWorkPackages(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doAddUser(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doUpdateUser(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doAddProject(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doRemoveUser(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doRemoveProject(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doAddWorkPackage(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doRemoveWorkPackage(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doAddToken(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doRemoveToken(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doGetTokens(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doAddDltFilter(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doRemoveDltFilter(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doSetDltFilterActive(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doGetDltFilters(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doAddSyslogFilter(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doRemoveSyslogFilter(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doSetSyslogFilterActive(const shared_ptr<Dispatcher> &mgr,
                                    const Dispatcher::Request &rq) -> bool;
static auto doGetSyslogFilters(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool;
static auto doQuit(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;
static auto doSendStatus(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool;

void Dispatcher::enableEvents()
{
    CDSApp()->addEventSource(m_queue);
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
    case Dispatcher::Action::TerminateServer:
        return doTerminateServer(getShared(), rq);
    case Dispatcher::Action::AuthUser:
        return doAuthUser(getShared(), rq);
    case Dispatcher::Action::AuthStatus:
        return doAuthStatus(getShared(), rq);
    case Dispatcher::Action::GetUsers:
        return doGetUsers(getShared(), rq);
    case Dispatcher::Action::GetProjects:
        return doGetProjects(getShared(), rq);
    case Dispatcher::Action::GetWorkPackages:
        return doGetWorkPackages(getShared(), rq);
    case Dispatcher::Action::AddUser:
        return doAddUser(getShared(), rq);
    case Dispatcher::Action::UpdateUser:
        return doUpdateUser(getShared(), rq);
    case Dispatcher::Action::AddProject:
        return doAddProject(getShared(), rq);
    case Dispatcher::Action::RemoveUser:
        return doRemoveUser(getShared(), rq);
    case Dispatcher::Action::RemoveProject:
        return doRemoveProject(getShared(), rq);
    case Dispatcher::Action::AddWorkPackage:
        return doAddWorkPackage(getShared(), rq);
    case Dispatcher::Action::RemoveWorkPackage:
        return doRemoveWorkPackage(getShared(), rq);
    case Dispatcher::Action::AddToken:
        return doAddToken(getShared(), rq);
    case Dispatcher::Action::RemoveToken:
        return doRemoveToken(getShared(), rq);
    case Dispatcher::Action::GetTokens:
        return doGetTokens(getShared(), rq);
    case Dispatcher::Action::AddDltFilter:
        return doAddDltFilter(getShared(), rq);
    case Dispatcher::Action::RemoveDltFilter:
        return doRemoveDltFilter(getShared(), rq);
    case Dispatcher::Action::SetDltFilterActive:
        return doSetDltFilterActive(getShared(), rq);
    case Dispatcher::Action::GetDltFilters:
        return doGetDltFilters(getShared(), rq);
    case Dispatcher::Action::AddSyslogFilter:
        return doAddSyslogFilter(getShared(), rq);
    case Dispatcher::Action::RemoveSyslogFilter:
        return doRemoveSyslogFilter(getShared(), rq);
    case Dispatcher::Action::SetSyslogFilterActive:
        return doSetSyslogFilterActive(getShared(), rq);
    case Dispatcher::Action::GetSyslogFilters:
        return doGetSyslogFilters(getShared(), rq);
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
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doTerminateServer(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    // TODO: Do some cleanup like closing all client connections
    Dispatcher::Request quitRq {.action = Dispatcher::Action::Quit};
    return mgr->pushRequest(quitRq);
}

static auto doGetUsers(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {
        .client = rq.client, .action = IDatabase::Action::GetUsers, .args = rq.args};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doAuthStatus(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    cds::msg::Envelope requestEnvelope;
    cds::msg::server::Message requestMessage;
    cds::msg::server::AuthStatus authStatus;

    if (rq.args.at(Defaults::Arg::Status) == cdsDefaults.valFor(Defaults::Val::StatusOkay)) {
        authStatus.set_status(cds::msg::server::AuthStatus::Status::AuthStatus_Status_Accepted);
    } else {
        authStatus.set_status(cds::msg::server::AuthStatus::Status::AuthStatus_Status_Rejected);
    }
    authStatus.set_info(rq.args.at(Defaults::Arg::Reason));

    requestMessage.set_type(cds::msg::server::Message_Type_AuthStatus);
    requestMessage.mutable_data()->PackFrom(authStatus);

    requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
    requestEnvelope.set_target(cds::msg::Envelope_Recipient_Client);
    requestEnvelope.set_origin(cds::msg::Envelope_Recipient_Server);

    logDebug() << "Send auth status to client " << rq.client->getName();
    auto status = rq.client->writeEnvelope(requestEnvelope);

    if (!status
        || (authStatus.status()
            == cds::msg::server::AuthStatus::Status::AuthStatus_Status_Rejected)) {
        logDebug() << "Client not authorized. Disconnect " << rq.client->getFD();
        CDSApp()->remEventSource(rq.client);
    }

    // We set the authentificated user now for this client. All sesitive requests should check the
    // authUser().
    rq.client->setAuthentificated(rq.args.at(Defaults::Arg::Owner));
    return true;
}

static auto doAuthUser(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::AuthUser,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doGetProjects(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {
        .client = rq.client, .action = IDatabase::Action::GetProjects, .args = rq.args};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doGetWorkPackages(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    IDatabase::Request dbrq {
        .client = rq.client, .action = IDatabase::Action::GetWorkPackages, .args = rq.args};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doAddUser(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::AddUser,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doUpdateUser(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::UpdateUser,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doAddProject(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::AddProject,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doRemoveUser(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {
        .client = rq.client, .action = IDatabase::Action::RemoveUser, .args = rq.args};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doRemoveProject(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    IDatabase::Request dbrq {
        .client = rq.client, .action = IDatabase::Action::RemoveProject, .args = rq.args};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doAddWorkPackage(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::AddWorkPackage,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doRemoveWorkPackage(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    IDatabase::Request dbrq {
        .client = rq.client, .action = IDatabase::Action::RemoveWorkPackage, .args = rq.args};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doAddToken(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::AddToken,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doRemoveToken(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::RemoveToken,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doGetTokens(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::GetTokens,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doAddDltFilter(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::AddDltFilter,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doRemoveDltFilter(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::RemoveDltFilter,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doSetDltFilterActive(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::SetDltFilterActive,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doGetDltFilters(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::GetDltFilters,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doAddSyslogFilter(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::AddSyslogFilter,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doRemoveSyslogFilter(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::RemoveSyslogFilter,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doSetSyslogFilterActive(const shared_ptr<Dispatcher> &mgr,
                                    const Dispatcher::Request &rq) -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::SetSyslogFilterActive,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
}

static auto doGetSyslogFilters(const shared_ptr<Dispatcher> &mgr, const Dispatcher::Request &rq)
    -> bool
{
    IDatabase::Request dbrq {.client = rq.client,
                             .action = IDatabase::Action::GetSyslogFilters,
                             .args = rq.args,
                             .bulkData = rq.bulkData};
    return CDSApp()->getDatabase()->pushRequest(dbrq);
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

    cds::msg::Envelope envelope;
    cds::msg::server::Message message;
    cds::msg::server::Status status;

    if (rq.args.count(Defaults::Arg::RequestId)) {
        status.set_requestid(rq.args.at(Defaults::Arg::RequestId));
    }

    if (rq.args.count(Defaults::Arg::Status)) {
        if (rq.args.at(Defaults::Arg::Status) == cdsDefaults.valFor(Defaults::Val::StatusOkay)) {
            status.set_what(cds::msg::server::Status_What_OK);
        } else if (rq.args.at(Defaults::Arg::Status)
                   == cdsDefaults.valFor(Defaults::Val::StatusBusy)) {
            status.set_what(cds::msg::server::Status_What_Busy);
        } else {
            status.set_what(cds::msg::server::Status_What_Error);
        }
    }

    if (rq.args.count(Defaults::Arg::Reason)) {
        status.set_reason(rq.args.at(Defaults::Arg::Reason));
    }

    // As response to client registration request we ask client to send descriptor
    message.set_type(cds::msg::server::Message_Type_Status);
    message.mutable_data()->PackFrom(status);
    envelope.mutable_mesg()->PackFrom(message);

    envelope.set_target(cds::msg::Envelope_Recipient_Any);
    envelope.set_origin(cds::msg::Envelope_Recipient_Server);

    logDebug() << "Send status " << rq.args.at(Defaults::Arg::Status) << " to "
               << rq.client->getFD();
    return rq.client->writeEnvelope(envelope);
}

} // namespace cds::server

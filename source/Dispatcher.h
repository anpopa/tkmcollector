/*-
 * Copyright (c) 2020 Alin Popa
 * All rights reserved.
 */

/*
 * @author Alin Popa <alin.popa@fxdata.ro>
 */

#pragma once

#include <map>
#include <string>

#include "Defaults.h"
#include "IClient.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"

using namespace bswi::event;

namespace cds::server
{

class Dispatcher : public std::enable_shared_from_this<Dispatcher>
{
public:
    enum class Action {
        InitDatabase,
        TerminateServer,
        AuthUser,
        AuthStatus,
        GetUsers,
        GetProjects,
        GetWorkPackages,
        AddUser,
        UpdateUser,
        AddProject,
        RemoveUser,
        RemoveProject,
        AddWorkPackage,
        RemoveWorkPackage,
        AddToken,
        RemoveToken,
        GetTokens,
        AddDltFilter,
        RemoveDltFilter,
        SetDltFilterActive,
        AddSyslogFilter,
        RemoveSyslogFilter,
        SetSyslogFilterActive,
        GetDltFilters,
        GetSyslogFilters,
        SendStatus,
        Quit
    };

    typedef struct Request {
        std::shared_ptr<IClient> client;
        Action action;
        std::map<Defaults::Arg, std::string> args;
        std::any bulkData;
    } Request;

public:
    Dispatcher()
    {
        m_queue = std::make_shared<AsyncQueue<Request>>(
            "DispatcherQueue", [this](const Request &rq) { return requestHandler(rq); });
    }

    auto getShared() -> std::shared_ptr<Dispatcher> { return shared_from_this(); }
    void enableEvents();
    auto pushRequest(Request &request) -> bool;

private:
    auto requestHandler(const Request &request) -> bool;

private:
    std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
};

} // namespace cds::server

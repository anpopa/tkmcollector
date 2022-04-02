#pragma once

#include <map>
#include <string>

#include "Connection.h"
#include "Defaults.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/Logger.h"

namespace tkm::control
{

class Dispatcher : public std::enable_shared_from_this<Dispatcher>
{
public:
    enum class Action {
        Connect,
        SendDescriptor,
        RequestSession,
        SetSession,
        InitDatabase,
        QuitCollector,
        GetDevices,
        AddDevice,
        RemoveDevice,
        ConnectDevice,
        DisconnectDevice,
        StartCollecting,
        StopCollecting,
        CollectorStatus,
        DeviceList,
        Quit
    };

    typedef struct Request {
        Action action;
        std::any bulkData;
        std::map<Defaults::Arg, std::string> args;
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

} // namespace tkm::control

#pragma once

#include <map>
#include <memory>
#include <string>

#include "Defaults.h"
#include "IClient.h"

#include "../bswinfra/source/AsyncQueue.h"

using namespace bswi::event;

namespace tkm::collector
{

class IDatabase
{
public:
    enum class Action {
        CheckDatabase,
        InitDatabase,
        Connect,
        Disconnect,
        GetDevices,
        AddDevice,
        RemoveDevice,
        LoadDevices,
        GetSessions,
        AddSession,
        RemSession,
        EndSession,
        CleanSessions,
        AddData
    };

    typedef struct Request {
        std::shared_ptr<IClient> client;
        Action action;
        std::map<Defaults::Arg, std::string> args;
        std::any bulkData;
    } Request;

public:
    explicit IDatabase()
    {
        m_queue = std::make_shared<AsyncQueue<IDatabase::Request>>(
            "DBQueue", [this](const IDatabase::Request &rq) { return requestHandler(rq); });
    }

    auto pushRequest(Request &rq) -> bool { return m_queue->push(rq); }
    virtual void enableEvents() = 0;
    virtual auto requestHandler(const IDatabase::Request &request) -> bool = 0;

public:
    IDatabase(IDatabase const &) = delete;
    void operator=(IDatabase const &) = delete;

protected:
    std::shared_ptr<AsyncQueue<IDatabase::Request>> m_queue = nullptr;
};

} // namespace tkm::collector

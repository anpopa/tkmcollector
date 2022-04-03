/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     IDevice Class
 * @details   Device interface
 *-
 */

#pragma once

#include <map>
#include <string>

#include "Defaults.h"
#include "IClient.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/Logger.h"

#include "Collector.pb.h"

using namespace bswi::event;

namespace tkm::collector
{

class IDevice
{
public:
    enum class Action {
        Connect,
        Disconnect,
        SendDescriptor,
        RequestSession,
        SetSession,
        StartStream,
        ProcessData,
        Status,
    };

    typedef struct Request {
        std::shared_ptr<IClient> client;
        Action action;
        std::map<Defaults::Arg, std::string> args;
        std::any bulkData;
    } Request;

public:
    IDevice()
    {
        m_queue = std::make_shared<AsyncQueue<Request>>(
            "DeviceQueue", [this](const Request &request) { return requestHandler(request); });
    }

    auto getDeviceData() -> tkm::msg::collector::DeviceData & { return m_deviceData; }
    auto getSessionData() -> tkm::msg::collector::SessionData & { return m_sessionData; }

    virtual auto pushRequest(Request &request) -> bool = 0;
    virtual void notifyConnection(tkm::msg::collector::DeviceData_State state) = 0;

protected:
    virtual auto requestHandler(const Request &request) -> bool = 0;

protected:
    std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
    tkm::msg::collector::DeviceData m_deviceData {};
    tkm::msg::collector::SessionData m_sessionData {};
};

} // namespace tkm::collector
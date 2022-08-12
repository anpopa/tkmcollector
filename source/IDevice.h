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
#include <taskmonitor/taskmonitor.h>

#include "Defaults.h"
#include "IClient.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/Timer.h"

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
    StartCollecting,
    StopCollecting,
    StartStream,
    StopStream,
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
  virtual ~IDevice() = default;

  virtual bool createConnection() = 0;
  virtual void enableConnection() = 0;
  virtual void deleteConnection() = 0;

  auto getDeviceData() -> tkm::msg::control::DeviceData & { return m_deviceData; }
  auto getSessionData() -> tkm::msg::control::SessionData & { return m_sessionData; }
  auto getSessionInfo() -> tkm::msg::monitor::SessionInfo & { return m_sessionInfo; }

  virtual bool pushRequest(Request &request) = 0;
  virtual void updateState(tkm::msg::control::DeviceData_State state) = 0;

protected:
  virtual bool requestHandler(const Request &request) = 0;

protected:
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  tkm::msg::control::DeviceData m_deviceData{};
  tkm::msg::control::SessionData m_sessionData{};
  tkm::msg::monitor::SessionInfo m_sessionInfo{};
};

} // namespace tkm::collector

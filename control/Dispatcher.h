/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Dispatcher Class
 * @details   Main application event dispatcher
 *-
 */

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
    GetSessions,
    RemoveSession,
    AddDevice,
    RemoveDevice,
    ConnectDevice,
    DisconnectDevice,
    StartCollecting,
    StopCollecting,
    CollectorStatus,
    DeviceList,
    SessionList,
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
  bool pushRequest(Request &request);

private:
  bool requestHandler(const Request &request);

private:
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
};

} // namespace tkm::control

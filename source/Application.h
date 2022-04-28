/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Application Class
 * @details   Main application class
 *-
 */

#pragma once

#include <atomic>
#include <cstdlib>
#include <string>

#include "DeviceManager.h"
#include "Dispatcher.h"
#include "IDatabase.h"
#include "Options.h"
#include "UDSServer.h"

#include "../bswinfra/source/EventLoop.h"
#include "../bswinfra/source/IApplication.h"

#include "Collector.pb.h"
#include "Control.pb.h"

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::collector
{

class Application : public bswi::app::IApplication
{
public:
  explicit Application(const std::string &name,
                       const std::string &description,
                       const std::string &configFile);

  static Application *getInstance()
  {
    return appInstance == nullptr
               ? appInstance = new Application("TKM-Collector",
                                               "TaskMonitor Collector Application",
                                               tkmDefaults.getFor(Defaults::Default::ConfPath))
               : appInstance;
  }

  void stop() final
  {
    if (m_running) {
      m_mainEventLoop->stop();
    }
  }
  auto getDispatcher() -> std::shared_ptr<Dispatcher> { return m_dispatcher; }
  auto getDatabase() -> std::shared_ptr<IDatabase> { return m_database; }
  auto getOptions() -> std::shared_ptr<Options> { return m_options; }
  auto getDeviceManager() -> std::shared_ptr<DeviceManager> { return m_deviceManager; }

public:
  Application(Application const &) = delete;
  void operator=(Application const &) = delete;

private:
  void startWatchdog(void);

private:
  std::shared_ptr<Options> m_options = nullptr;
  std::shared_ptr<UDSServer> m_udsServer = nullptr;
  std::shared_ptr<Dispatcher> m_dispatcher = nullptr;
  std::shared_ptr<IDatabase> m_database = nullptr;
  std::shared_ptr<DeviceManager> m_deviceManager = nullptr;

private:
  static Application *appInstance;
};

} // namespace tkm::collector

#define CollectorApp() tkm::collector::Application::getInstance()

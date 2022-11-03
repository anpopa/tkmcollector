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

#include <filesystem>
#include <stdexcept>

#include "Application.h"
#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif
#ifdef WITH_SQLITE3
#include "SQLiteDatabase.h"
#endif
#ifdef WITH_POSTGRESQL
#include "PQDatabase.h"
#endif

#define USEC2SEC(x) (x / 1000000)

namespace tkm::collector
{

Application *Application::appInstance = nullptr;

Application::Application(const std::string &name,
                         const std::string &description,
                         const std::string &configFile)
: bswi::app::IApplication(name, description)
{
  if (Application::appInstance != nullptr) {
    throw bswi::except::SingleInstance();
  }
  appInstance = this;

  // Create Options
  m_options = std::make_shared<Options>(configFile);

  // Create DeviceManager
  m_deviceManager = std::make_shared<DeviceManager>();

  // Create runtime directory
  std::filesystem::path runDir(m_options->getFor(Options::Key::RuntimeDirectory));
  if (!std::filesystem::exists(runDir)) {
    if (!std::filesystem::create_directories(runDir)) {
      throw std::runtime_error("Fail to create runtime directory");
    }
  }

  // Create Dispatcher
  m_dispatcher = std::make_shared<Dispatcher>();
  m_dispatcher->enableEvents();

  // Use one of DB backends
  if (m_options->getFor(Options::Key::DatabaseType) == "sqlite3") {
#ifdef WITH_SQLITE3
    try {
      m_database = std::make_shared<SQLiteDatabase>(m_options);
      m_database->enableEvents();
    } catch (std::exception &e) {
      logError() << "Fail to open database. Reason: " << e.what();
      std::cout << "Fail to open database. Reason: " << e.what() << std::endl;
      Dispatcher::Request rq{.client = nullptr,
                             .action = Dispatcher::Action::Quit,
                             .args = std::map<Defaults::Arg, std::string>(),
                             .bulkData = std::make_any<int>(0)};
      m_dispatcher->pushRequest(rq);
    }
#else
    static_assert(true, "SQLite3 database configured but support not enabled at build time");
#endif
  } else {
#ifdef WITH_POSTGRESQL
    try {
      m_database = std::make_shared<PQDatabase>(m_options);
      m_database->enableEvents();
    } catch (std::exception &e) {
      logError() << "Fail to open database. Reason: " << e.what();
      std::cout << "Fail to open database. Reason: " << e.what() << std::endl;
      Dispatcher::Request rq{.client = nullptr,
                             .action = Dispatcher::Action::Quit,
                             .args = std::map<Defaults::Arg, std::string>(),
                             .bulkData = std::make_any<int>(0)};
      m_dispatcher->pushRequest(rq);
    }
#else
    static_assert(true, "PostgreSQL database configured but support not enabled at build time");
#endif
  }

  // Create UDSServer
  m_udsServer = std::make_shared<UDSServer>();
  m_udsServer->enableEvents();

  // Start server interfaces
  try {
    m_udsServer->start();
  } catch (std::exception &e) {
    logError() << "Fail to start server. Exception: " << e.what();
    std::cout << "Failed to start server. Reason: " << e.what() << std::endl;
    Dispatcher::Request rq{.client = nullptr,
                           .action = Dispatcher::Action::Quit,
                           .args = std::map<Defaults::Arg, std::string>(),
                           .bulkData = std::make_any<int>(0)};
    m_dispatcher->pushRequest(rq);
  }

  if (m_database != nullptr) {
    // Load devices from database
    m_deviceManager->loadDevices();
    // Mark all in progress sessions as complete
    m_deviceManager->cleanSessions();
  }

  startWatchdog();
}

void Application::startWatchdog(void)
{
#ifdef WITH_SYSTEMD
  ulong usec = 0;
  int status;

  status = sd_watchdog_enabled(0, &usec);
  if (status > 0) {
    logInfo() << "Systemd watchdog enabled with timeout seconds: " << USEC2SEC(usec);

    auto timer = std::make_shared<Timer>("Watchdog", []() {
      if (sd_notify(0, "WATCHDOG=1") < 0) {
        logWarn() << "Fail to send the heartbeet to systemd";
      } else {
        logDebug() << "Watchdog heartbeat sent";
      }

      return true;
    });

    timer->start((usec / 2), true);
    CollectorApp()->addEventSource(timer);
  } else {
    if (status == 0) {
      logInfo() << "Systemd watchdog disabled";
    } else {
      logWarn() << "Fail to get the systemd watchdog status";
    }
  }
#else
  logInfo() << "Watchdog build time disabled";
#endif
}

} // namespace tkm::collector

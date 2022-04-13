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
#ifdef WITH_SQLITE3
#include "SQLiteDatabase.h"
#endif
#ifdef WITH_POSTGRESQL
#include "PQDatabase.h"
#endif

namespace fs = std::filesystem;
using std::shared_ptr;
using std::string;

namespace tkm::collector
{

Application *Application::appInstance = nullptr;

Application::Application(const string &name, const string &description, const string &configFile)
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
  fs::path runDir(m_options->getFor(Options::Key::RuntimeDirectory));
  if (!fs::exists(runDir)) {
    if (!fs::create_directories(runDir)) {
      throw std::runtime_error("Fail to create runtime directory");
    }
  }

  // Create Dispatcher
  m_dispatcher = std::make_shared<Dispatcher>();
  m_dispatcher->enableEvents();

  // Use one of DB backends
  if (m_options->getFor(Options::Key::RuntimeDirectory) == "sqlite3") {
#ifdef WITH_SQLITE3
    try {
      m_database = std::make_shared<SQLiteDatabase>(m_options);
      m_database->enableEvents();
    } catch (std::exception &e) {
      logError() << "Fail to open database. Reason: " << e.what();
      std::cout << "Fail to open database. Reason: " << e.what() << std::endl;
      Dispatcher::Request rq{.action = Dispatcher::Action::Quit};
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
      Dispatcher::Request rq{.action = Dispatcher::Action::Quit};
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
    Dispatcher::Request rq{.action = Dispatcher::Action::Quit};
    m_dispatcher->pushRequest(rq);
  }

  // Load devices from database
  m_deviceManager->loadDevices();
  // Mark all in progress sessions as complete
  m_deviceManager->cleanSessions();
}

} // namespace tkm::collector

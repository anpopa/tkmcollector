/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Application Class
 * @details   Main control application
 *-
 */

#include <filesystem>
#include <stdexcept>

#include "Application.h"

namespace tkm::control
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

  // Check runtime directory
  std::filesystem::path runDir(m_options->getFor(Options::Key::RuntimeDirectory));
  if (!std::filesystem::exists(runDir)) {
    throw std::runtime_error("Server runtime directory not available");
  }

  m_dispatcher = std::make_unique<Dispatcher>();
  m_dispatcher->enableEvents();

  m_connection = std::make_shared<Connection>();
  m_connection->enableEvents();

  m_command = std::make_unique<Command>();
  m_command->enableEvents();
}

} // namespace tkm::control

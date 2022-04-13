/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Options class
 * @details   Runtime options with defaults
 *-
 */

#include "Options.h"
#include "Defaults.h"

using namespace std;

namespace tkm
{

Options::Options(const string &configFile)
{
  m_configFile = std::make_shared<bswi::kf::KeyFile>(configFile);
  if (m_configFile->parseFile() != 0) {
    logWarn() << "Fail to parse config file: " << configFile;
    m_configFile.reset();
  }
}

auto Options::getFor(Key key) -> string
{
  switch (key) {
  case Key::DBName:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("database", -1, "DatabaseName");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBName));
    }
    return tkmDefaults.getFor(Defaults::Default::DBName);
  case Key::DBUserName:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("database", -1, "UserName");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBUserName));
    }
    return tkmDefaults.getFor(Defaults::Default::DBUserName);
  case Key::DBUserPassword:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("database", -1, "UserPassword");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBUserPassword));
    }
    return tkmDefaults.getFor(Defaults::Default::DBUserPassword);
  case Key::DBServerAddress:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("database", -1, "ServerAddress");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBServerAddress));
    }
    return tkmDefaults.getFor(Defaults::Default::DBServerAddress);
  case Key::DBServerPort:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("database", -1, "ServerPort");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBServerPort));
    }
    return tkmDefaults.getFor(Defaults::Default::DBServerPort);
  case Key::DBFilePath:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("database", -1, "DatabasePath");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBFilePath));
    }
    return tkmDefaults.getFor(Defaults::Default::DBFilePath);
  case Key::RuntimeDirectory:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("general", -1, "RuntimeDirectory");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::RuntimeDirectory));
    }
    return tkmDefaults.getFor(Defaults::Default::RuntimeDirectory);
  case Key::DatabaseType:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("general", -1, "DatabaseType");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::DatabaseType));
    }
    return tkmDefaults.getFor(Defaults::Default::DatabaseType);
  default:
    logError() << "Unknown option key";
    break;
  }

  throw std::runtime_error("Cannot provide option for key");
}

} // namespace tkm

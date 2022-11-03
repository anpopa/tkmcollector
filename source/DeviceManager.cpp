/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     DeviceManager Class
 * @details   Manage IDevice objects
 *-
 */

#include "DeviceManager.h"
#include "Application.h"
#include "IDatabase.h"

namespace tkm::collector
{

bool DeviceManager::addDevice(std::shared_ptr<MonitorDevice> device)
{
  auto found = false;

  m_devices.foreach ([this, &device, &found](const std::shared_ptr<MonitorDevice> entry) {
    if (entry->getDeviceData().hash() == device->getDeviceData().hash()) {
      found = true;
    }
  });

  if (!found) {
    m_devices.append(device);
    m_devices.commit();
  }

  return !found;
}

bool DeviceManager::remDevice(std::shared_ptr<MonitorDevice> device)
{
  auto found = false;

  m_devices.foreach ([this, &device, &found](const std::shared_ptr<MonitorDevice> entry) {
    if (entry->getDeviceData().hash() == device->getDeviceData().hash()) {
      logDebug() << "Found device to remove with hash " << entry->getDeviceData().hash();
      entry->getConnection()->disconnect();
      m_devices.remove(entry);
      found = true;
    }
  });
  m_devices.commit();

  return found;
}

auto DeviceManager::getDevice(const std::string &hash) -> std::shared_ptr<MonitorDevice>
{
  std::shared_ptr<MonitorDevice> retEntry = nullptr;

  m_devices.foreach ([&hash, &retEntry](const std::shared_ptr<MonitorDevice> entry) {
    if (entry->getDeviceData().hash() == hash) {
      retEntry = entry;
    }
  });

  return retEntry;
}

bool DeviceManager::loadDevices(void)
{
  IDatabase::Request dbrq{.client = nullptr,
                          .action = IDatabase::Action::LoadDevices,
                          .args = std::map<Defaults::Arg, std::string>(),
                          .bulkData = std::make_any<int>(0)};
  return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

bool DeviceManager::cleanSessions(void)
{
  IDatabase::Request dbrq{.client = nullptr,
                          .action = IDatabase::Action::CleanSessions,
                          .args = std::map<Defaults::Arg, std::string>(),
                          .bulkData = std::make_any<int>(0)};
  return CollectorApp()->getDatabase()->pushRequest(dbrq);
}

} // namespace tkm::collector

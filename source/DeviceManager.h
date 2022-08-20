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

#pragma once

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

#include "MonitorDevice.h"
#include "Options.h"

#include "../bswinfra/source/Pollable.h"
#include "../bswinfra/source/SafeList.h"

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::collector
{

class DeviceManager : public std::enable_shared_from_this<DeviceManager>
{
public:
  DeviceManager() = default;
  ~DeviceManager() = default;

public:
  DeviceManager(DeviceManager const &) = delete;
  void operator=(DeviceManager const &) = delete;

  bool hasDevices(void) { return (m_devices.getSize() > 0); }
  bool loadDevices(void);
  bool cleanSessions(void);

  bool addDevice(std::shared_ptr<MonitorDevice> device);
  bool remDevice(std::shared_ptr<MonitorDevice> device);
  auto getDevice(const std::string &hash) -> std::shared_ptr<MonitorDevice>;

private:
  bswi::util::SafeList<std::shared_ptr<MonitorDevice>> m_devices{"DeviceList"};
};

} // namespace tkm::collector

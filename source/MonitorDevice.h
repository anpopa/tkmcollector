/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     MonitorDevice Class
 * @details   Device object associated with a device entry
 *-
 */

#pragma once

#include <map>
#include <string>

#include "Connection.h"
#include "IDevice.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/Logger.h"

#include "Collector.pb.h"

using namespace bswi::event;

namespace tkm::collector
{

class MonitorDevice : public IDevice, public std::enable_shared_from_this<MonitorDevice>
{
public:
  explicit MonitorDevice(const tkm::msg::collector::DeviceData &data)
  {
    m_deviceData.CopyFrom(data);
  }
  ~MonitorDevice() = default;

  bool createConnection() final;
  void enableConnection() final;
  void deleteConnection() final;

  auto getShared() -> std::shared_ptr<MonitorDevice> { return shared_from_this(); }
  auto getConnection() -> std::shared_ptr<Connection> & { return m_connection; }
  void enableEvents();
  bool pushRequest(Request &request);
  void updateState(tkm::msg::collector::DeviceData_State state) final;

private:
  bool requestHandler(const Request &request) final;

private:
  std::shared_ptr<Connection> m_connection = nullptr;
};

} // namespace tkm::collector
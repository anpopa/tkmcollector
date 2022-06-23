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
#include "../bswinfra/source/Timer.h"

#include "Control.pb.h"

using namespace bswi::event;

namespace tkm::collector
{

class MonitorDevice : public IDevice, public std::enable_shared_from_this<MonitorDevice>
{
public:
  explicit MonitorDevice(const tkm::msg::control::DeviceData &data)
  {
    m_deviceData.CopyFrom(data);
    initTimers();
  }
  ~MonitorDevice() = default;

  bool createConnection() final;
  void enableConnection() final;
  void deleteConnection() final;

  auto getShared() -> std::shared_ptr<MonitorDevice> { return shared_from_this(); }
  auto getConnection() -> std::shared_ptr<Connection> { return m_connection; }
  void enableEvents();
  bool pushRequest(Request &request) override;
  void updateState(tkm::msg::control::DeviceData_State state) final;

  auto getProcAcctTimer() -> std::shared_ptr<Timer> { return m_procAcctTimer; }
  auto getProcInfoTimer() -> std::shared_ptr<Timer> { return m_procInfoTimer; }
  auto getContextInfoTimer() -> std::shared_ptr<Timer> { return m_contextInfoTimer; }
  auto getProcEventTimer() -> std::shared_ptr<Timer> { return m_procEventTimer; }
  auto getSysProcStatTimer() -> std::shared_ptr<Timer> { return m_sysProcStatTimer; }
  auto getSysProcMemInfoTimer() -> std::shared_ptr<Timer> { return m_sysProcMemInfoTimer; }
  auto getSysProcPressureTimer() -> std::shared_ptr<Timer> { return m_sysProcPressureTimer; }
  auto getSysProcDiskStatsTimer() -> std::shared_ptr<Timer> { return m_sysProcDiskStatsTimer; }

private:
  bool requestHandler(const Request &request) final;
  void initTimers(void);

private:
  std::shared_ptr<Connection> m_connection = nullptr;
  std::shared_ptr<Timer> m_procAcctTimer = nullptr;
  std::shared_ptr<Timer> m_procInfoTimer = nullptr;
  std::shared_ptr<Timer> m_contextInfoTimer = nullptr;
  std::shared_ptr<Timer> m_procEventTimer = nullptr;
  std::shared_ptr<Timer> m_sysProcStatTimer = nullptr;
  std::shared_ptr<Timer> m_sysProcMemInfoTimer = nullptr;
  std::shared_ptr<Timer> m_sysProcPressureTimer = nullptr;
  std::shared_ptr<Timer> m_sysProcDiskStatsTimer = nullptr;
};

} // namespace tkm::collector

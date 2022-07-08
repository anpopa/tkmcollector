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
#include "DataSource.h"
#include "IDevice.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/SafeList.h"
#include "../bswinfra/source/Timer.h"

#include "Control.pb.h"

using namespace bswi::event;

namespace tkm::collector
{

class MonitorDevice : public IDevice, public std::enable_shared_from_this<MonitorDevice>
{
public:
  explicit MonitorDevice(const tkm::msg::control::DeviceData &data) { m_deviceData.CopyFrom(data); }
  ~MonitorDevice() = default;

  bool createConnection() final;
  void enableConnection() final;
  void deleteConnection() final;

  auto getShared() -> std::shared_ptr<MonitorDevice> { return shared_from_this(); }
  auto getConnection() -> std::shared_ptr<Connection> { return m_connection; }
  void enableEvents();
  bool pushRequest(Request &request) override;
  void updateState(tkm::msg::control::DeviceData_State state) final;

  void startUpdateLanes(void);
  void stopUpdateLanes(void);

private:
  bool requestHandler(const Request &request) final;
  void configUpdateLanes(void);

private:
  bswi::util::SafeList<std::shared_ptr<DataSource>> m_dataSources{"DataSourceList"};
  std::shared_ptr<Connection> m_connection = nullptr;
  std::shared_ptr<Timer> m_fastLaneTimer = nullptr;
  std::shared_ptr<Timer> m_paceLaneTimer = nullptr;
  std::shared_ptr<Timer> m_slowLaneTimer = nullptr;
};

} // namespace tkm::collector

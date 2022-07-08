/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     DataSource Class
 * @details   Data source
 *-
 */

#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace tkm::collector
{

class DataSource
{
public:
  typedef enum _UpdateLane { Fast, Pace, Slow, Any } UpdateLane;

public:
  explicit DataSource(const std::string &name,
                      UpdateLane lane,
                      const std::function<bool()> &callback)
  : m_lane(lane)
  , m_callback(callback)
  {
    std::string laneName{"Any"};

    switch (lane) {
    case UpdateLane::Fast:
      laneName = "Fast";
      break;
    case UpdateLane::Pace:
      laneName = "Pace";
      break;
    case UpdateLane::Slow:
      laneName = "Slow";
      break;
    default:
      break;
    }

    logDebug() << "New data source name='" << name << "' lane='" << laneName << "'";
  };
  ~DataSource() = default;

public:
  DataSource(DataSource const &) = delete;
  void operator=(DataSource const &) = delete;

public:
  auto getUpdateLane(void) -> UpdateLane { return m_lane; }
  bool update(void)
  {
    if (m_callback != nullptr) {
      return m_callback();
    }
    return false;
  };

private:
  UpdateLane m_lane = UpdateLane::Any;
  std::function<bool()> m_callback = nullptr;
};

} // namespace tkm::collector

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
    MonitorDevice() { m_connection = std::make_shared<Connection>(getShared()); }

    auto getShared() -> std::shared_ptr<MonitorDevice> { return shared_from_this(); }
    auto getConnection() -> std::shared_ptr<Connection> { return m_connection; }
    void enableEvents();
    auto pushRequest(Request &request) -> bool;
    void notifyConnection(tkm::msg::collector::DeviceData_State state) final;

private:
    auto requestHandler(const Request &request) -> bool final;

private:
    std::shared_ptr<Connection> m_connection = nullptr;
};

} // namespace tkm::collector
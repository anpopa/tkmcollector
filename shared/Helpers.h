/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Helper methods
 * @details   Verious helper methods
 *-
 */

#pragma once

#include <any>
#include <string>

#include <taskmonitor/taskmonitor.h>

namespace tkm
{

auto hashForDevice(const tkm::msg::control::DeviceData &data) -> std::string;
bool sendControlDescriptor(int fd, tkm::msg::control::Descriptor &descriptor);
bool readControlDescriptor(int fd, tkm::msg::control::Descriptor &descriptor);

} // namespace tkm

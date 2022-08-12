/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Connection Class
 * @details   Handle connection to a taskmonitor device
 *-
 */

#pragma once

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

#include "Helpers.h"
#include "IDevice.h"
#include "Options.h"
#include <taskmonitor/taskmonitor.h>

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/Logger.h"
#include "../bswinfra/source/Pollable.h"

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::collector
{

class Connection final : public Pollable, public std::enable_shared_from_this<Connection>
{
public:
  explicit Connection(std::shared_ptr<IDevice> device);
  ~Connection();

public:
  Connection(Connection const &) = delete;
  void operator=(Connection const &) = delete;

  void enableEvents();
  auto connect() -> int;
  void disconnect();
  [[nodiscard]] int getFD() const { return m_sockFd; }
  auto getShared() -> std::shared_ptr<Connection> { return shared_from_this(); }

  auto readEnvelope(tkm::msg::Envelope &envelope) -> IAsyncEnvelope::Status
  {
    return m_reader->next(envelope);
  }

  bool writeEnvelope(const tkm::msg::Envelope &envelope)
  {
    if (m_writer->send(envelope) == IAsyncEnvelope::Status::Ok) {
      return m_writer->flush();
    }
    return true;
  }

private:
  std::shared_ptr<IDevice> m_device = nullptr;
  std::unique_ptr<EnvelopeReader> m_reader = nullptr;
  std::unique_ptr<EnvelopeWriter> m_writer = nullptr;
  struct sockaddr_in m_addr = {};
  int m_sockFd = -1;
};

} // namespace tkm::collector

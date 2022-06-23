/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     IClient Class
 * @details   Interfaces for UDS clients
 *-
 */

#pragma once

#include "EnvelopeReader.h"
#include "EnvelopeWriter.h"
#include "Options.h"

#include <fcntl.h>
#include <memory>
#include <mutex>
#include <unistd.h>

#include "../bswinfra/source/Pollable.h"

using namespace bswi::event;

namespace tkm::collector
{

class IClient : public Pollable
{
public:
  explicit IClient(const std::string &name, int fd)
  : Pollable(name, fd)
  , m_reader(std::make_unique<EnvelopeReader>(fd))
  , m_writer(std::make_unique<EnvelopeWriter>(fd))
  {
  }

  virtual ~IClient() { disconnect(); }

  void disconnect()
  {
    if (m_fd > 0) {
      ::close(m_fd);
      m_fd = -1;
    }
  }

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

public:
  IClient(IClient const &) = delete;
  void operator=(IClient const &) = delete;

  [[nodiscard]] int getFD() const { return m_fd; }

private:
  std::unique_ptr<EnvelopeReader> m_reader = nullptr;
  std::unique_ptr<EnvelopeWriter> m_writer = nullptr;
};

} // namespace tkm::collector

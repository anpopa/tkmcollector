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

namespace tkm::collector {

class IClient : public Pollable {
public:
  explicit IClient(const std::string &name, int fd)
      : Pollable(name), m_clientFd(fd) {
    m_reader = std::make_unique<EnvelopeReader>(fd);
    m_writer = std::make_unique<EnvelopeWriter>(fd);
  }

  ~IClient() { disconnect(); }

  void disconnect() {
    if (m_clientFd > 0) {
      ::close(m_clientFd);
      m_clientFd = -1;
    }
  }

  auto readEnvelope(tkm::msg::Envelope &envelope) -> IAsyncEnvelope::Status {
    return m_reader->next(envelope);
  }
  auto writeEnvelope(const tkm::msg::Envelope &envelope) -> bool {
    if (m_writer->send(envelope) == IAsyncEnvelope::Status::Ok) {
      return m_writer->flush();
    }
    return true;
  }

public:
  IClient(IClient const &) = delete;
  void operator=(IClient const &) = delete;

  [[nodiscard]] int getFD() const { return m_clientFd; }

private:
  std::unique_ptr<EnvelopeReader> m_reader = nullptr;
  std::unique_ptr<EnvelopeWriter> m_writer = nullptr;

protected:
  int m_clientFd = -1;
};

} // namespace tkm::collector

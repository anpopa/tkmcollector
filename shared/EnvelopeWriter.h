#pragma once

#include <string>

#include "Envelope.pb.h"
#include "IAsyncEnvelope.h"

namespace tkm
{

class EnvelopeWriter : public IAsyncEnvelope, public std::enable_shared_from_this<EnvelopeWriter>
{
public:
    explicit EnvelopeWriter(int fd);

    auto send(const tkm::msg::Envelope &envelope) -> IAsyncEnvelope::Status;
    auto flush() -> bool;

public:
    EnvelopeWriter(EnvelopeWriter const &) = delete;
    void operator=(EnvelopeWriter const &) = delete;

private:
    auto flushInternal() -> bool;
};

} // namespace tkm

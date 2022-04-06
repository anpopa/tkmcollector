/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Connection Class
 * @details   Manage connection with collector
 *-
 */

#pragma once

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

#include "EnvelopeReader.h"
#include "EnvelopeWriter.h"
#include "Helpers.h"
#include "Options.h"

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/IApplication.h"
#include "../bswinfra/source/Pollable.h"

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::control
{

class Connection : public Pollable, public std::enable_shared_from_this<Connection>
{
public:
    Connection();
    ~Connection();

    auto getShared() -> std::shared_ptr<Connection> { return shared_from_this(); }
    void enableEvents();
    auto connect() -> int;
    [[nodiscard]] int getFD() const { return m_sockFd; }

    auto readEnvelope(tkm::msg::Envelope &envelope) -> IAsyncEnvelope::Status
    {
        return m_reader->next(envelope);
    }

    auto writeEnvelope(const tkm::msg::Envelope &envelope) -> bool
    {
        if (m_writer->send(envelope) == IAsyncEnvelope::Status::Ok) {
            return m_writer->flush();
        }
        return true;
    }

public:
    Connection(Connection const &) = delete;
    void operator=(Connection const &) = delete;

private:
    std::unique_ptr<EnvelopeReader> m_reader = nullptr;
    std::unique_ptr<EnvelopeWriter> m_writer = nullptr;
    struct sockaddr_un m_addr = {};
    int m_sockFd = -1;
};

} // namespace tkm::control

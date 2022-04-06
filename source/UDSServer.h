/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     UDSServer Class
 * @details   Unix domain server connection for ControlClient
 *-
 */

#pragma once

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

#include "Options.h"

#include "../bswinfra/source/Pollable.h"

#include "Collector.pb.h"

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::collector
{

class UDSServer : public Pollable, public std::enable_shared_from_this<UDSServer>
{
public:
    UDSServer();
    ~UDSServer();

    auto getShared() -> std::shared_ptr<UDSServer> { return shared_from_this(); }
    void enableEvents();
    void start();
    void stop();

public:
    UDSServer(UDSServer const &) = delete;
    void operator=(UDSServer const &) = delete;

private:
    struct sockaddr_un m_addr {
    };
    int m_sockFd = -1;
};

} // namespace tkm::collector

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

#include <filesystem>
#include <unistd.h>

#include "Application.h"
#include "ControlClient.h"
#include "Helpers.h"
#include "UDSServer.h"

namespace fs = std::filesystem;
using std::shared_ptr;
using std::string;

namespace tkm::collector
{

UDSServer::UDSServer()
: Pollable("UDSServer")
{
  if ((m_sockFd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    throw std::runtime_error("Fail to create UDSServer socket");
  }

  lateSetup(
      [this]() {
        int clientFd = accept(m_sockFd, (struct sockaddr *) nullptr, nullptr);

        if (clientFd < 0) {
          logWarn() << "Fail to accept on UDSServer socket";
          return false;
        }

        // The client has 3 seconds to send the descriptor or will be
        // disconnected
        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        setsockopt(clientFd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));

        tkm::msg::collector::Descriptor descriptor{};

        if (!readControlDescriptor(clientFd, descriptor)) {
          logWarn() << "Control client " << clientFd << " read descriptor failed";
          close(clientFd);
          return true; // this is a client issue, process next client
        }

        logInfo() << "New ControlClient with FD: " << clientFd;
        std::shared_ptr<ControlClient> client = std::make_shared<ControlClient>(clientFd);
        client->enableEvents();

        return true;
      },
      m_sockFd,
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  // We are ready for events only after start
  setPrepare([]() { return false; });
}

void UDSServer::enableEvents()
{
  CollectorApp()->addEventSource(getShared());
}

UDSServer::~UDSServer()
{
  static_cast<void>(stop());
}

void UDSServer::start()
{
  fs::path sockPath(CollectorApp()->getOptions()->getFor(Options::Key::RuntimeDirectory));
  sockPath /= tkmDefaults.getFor(Defaults::Default::ControlSocket);

  m_addr.sun_family = AF_UNIX;
  strncpy(m_addr.sun_path, sockPath.c_str(), sizeof(m_addr.sun_path));

  if (fs::exists(sockPath)) {
    logWarn() << "Runtime directory not clean, removing " << sockPath.string();
    if (!fs::remove(sockPath)) {
      throw std::runtime_error("Fail to remove existing UDSServer socket");
    }
  }

  if (bind(m_sockFd, (struct sockaddr *) &m_addr, sizeof(struct sockaddr_un)) != -1) {
    // We are ready for events only after start
    setPrepare([]() { return true; });
    if (listen(m_sockFd, 10) == -1) {
      logError() << "UDSServer listening failed on " << sockPath.string()
                 << ". Error: " << strerror(errno);
      throw std::runtime_error("UDSServer server listen failed");
    }
    logInfo() << "Control server listening on " << sockPath.string();
  } else {
    logError() << "UDSServer bind failed on " << sockPath.string()
               << ". Error: " << strerror(errno);
    throw std::runtime_error("UDSServer server bind failed");
  }
}

void UDSServer::stop()
{
  if (m_sockFd > 0) {
    ::close(m_sockFd);
  }
}

} // namespace tkm::collector

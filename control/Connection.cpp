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

#include <csignal>
#include <fcntl.h>
#include <filesystem>
#include <taskmonitor/taskmonitor.h>
#include <unistd.h>

#include "Application.h"
#include "Connection.h"
#include "Defaults.h"
#include "Helpers.h"

namespace tkm::control
{

Connection::Connection()
: Pollable("Connection")
{
  if ((m_sockFd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    throw std::runtime_error("Fail to create Connection socket");
  }

  m_reader = std::make_unique<EnvelopeReader>(m_sockFd);
  m_writer = std::make_unique<EnvelopeWriter>(m_sockFd);

  lateSetup(
      [this]() {
        auto status = true;

        do {
          tkm::msg::Envelope envelope;

          // Read next message
          auto readStatus = readEnvelope(envelope);
          if (readStatus == IAsyncEnvelope::Status::Again) {
            return true;
          } else if (readStatus == IAsyncEnvelope::Status::Error) {
            logDebug() << "Control read error";
            return false;
          } else if (readStatus == IAsyncEnvelope::Status::EndOfFile) {
            logDebug() << "Control read end of file";
            return false;
          }

          // Check for valid origin
          if (envelope.origin() != tkm::msg::Envelope_Recipient_Collector) {
            continue;
          }

          tkm::msg::control::Message msg;
          envelope.mesg().UnpackTo(&msg);

          switch (msg.type()) {
          case tkm::msg::control::Message_Type_SetSession: {
            Dispatcher::Request rq{.action = Dispatcher::Action::SetSession,
                                   .bulkData = std::make_any<int>(0),
                                   .args = std::map<Defaults::Arg, std::string>()};
            tkm::msg::control::SessionInfo sessionInfo;

            msg.data().UnpackTo(&sessionInfo);
            rq.bulkData = std::make_any<tkm::msg::control::SessionInfo>(sessionInfo);

            ControlApp()->getDispatcher()->pushRequest(rq);
            break;
          }
          case tkm::msg::control::Message_Type_Status: {
            Dispatcher::Request rq{.action = Dispatcher::Action::CollectorStatus,
                                   .bulkData = std::make_any<int>(0),
                                   .args = std::map<Defaults::Arg, std::string>()};
            tkm::msg::control::Status data;

            msg.data().UnpackTo(&data);
            rq.bulkData = std::make_any<tkm::msg::control::Status>(data);

            ControlApp()->getDispatcher()->pushRequest(rq);
            break;
          }
          case tkm::msg::control::Message_Type_DeviceList: {
            Dispatcher::Request rq{.action = Dispatcher::Action::DeviceList,
                                   .bulkData = std::make_any<int>(0),
                                   .args = std::map<Defaults::Arg, std::string>()};
            tkm::msg::control::DeviceList deviceList;

            msg.data().UnpackTo(&deviceList);
            rq.bulkData = std::make_any<tkm::msg::control::DeviceList>(deviceList);

            ControlApp()->getDispatcher()->pushRequest(rq);
            break;
          }
          case tkm::msg::control::Message_Type_SessionList: {
            Dispatcher::Request rq{.action = Dispatcher::Action::SessionList,
                                   .bulkData = std::make_any<int>(0),
                                   .args = std::map<Defaults::Arg, std::string>()};
            tkm::msg::control::SessionList sessionList;

            msg.data().UnpackTo(&sessionList);
            rq.bulkData = std::make_any<tkm::msg::control::SessionList>(sessionList);

            ControlApp()->getDispatcher()->pushRequest(rq);
            break;
          }
          default:
            logError() << "Unknown response type";
            status = false;
            break;
          }
        } while (status);

        return status;
      },
      m_sockFd,
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  // We are ready for events only after connect
  setPrepare([]() { return false; });
  // If the event is removed we stop the main application
  setFinalize([]() {
    logInfo() << "Server closed connection. Terminate";
    Dispatcher::Request nrq{.action = Dispatcher::Action::Quit,
                            .bulkData = std::make_any<int>(0),
                            .args = std::map<Defaults::Arg, std::string>()};
    ControlApp()->getDispatcher()->pushRequest(nrq);
  });
}

void Connection::enableEvents()
{
  ControlApp()->addEventSource(getShared());
}

Connection::~Connection()
{
  if (m_sockFd > 0) {
    ::close(m_sockFd);
  }
}

auto Connection::connect() -> int
{
  std::filesystem::path sockPath(
      ControlApp()->getOptions()->getFor(Options::Key::RuntimeDirectory));
  sockPath /= tkmDefaults.getFor(Defaults::Default::ControlSocket);

  m_addr.sun_family = AF_UNIX;
  strncpy(m_addr.sun_path, sockPath.c_str(), sizeof(m_addr.sun_path) - 1);

  if (!std::filesystem::exists(sockPath)) {
    throw std::runtime_error("Collector IPC socket not available");
  }

  if (::connect(m_sockFd, (struct sockaddr *) &m_addr, sizeof(struct sockaddr_un)) == -1) {
    logError() << "Failed to connect to server";
    return -1;
  }

  // We are ready to process events
  logInfo() << "Connected to server";
  setPrepare([]() { return true; });

  return 0;
}

} // namespace tkm::control

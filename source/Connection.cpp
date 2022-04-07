/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Command Class
 * @details   Handle user command sequence
 *-
 */

#include <csignal>
#include <errno.h>
#include <filesystem>
#include <netdb.h>
#include <string>
#include <unistd.h>

#include "Application.h"
#include "Connection.h"
#include "Defaults.h"
#include "Helpers.h"

#include "Server.pb.h"

namespace fs = std::filesystem;

namespace tkm::collector
{

Connection::Connection(std::shared_ptr<IDevice> device)
: Pollable("DeviceConnection")
, m_device(device)
{
    if ((m_sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
                    logDebug() << "Read error";
                    return false;
                } else if (readStatus == IAsyncEnvelope::Status::EndOfFile) {
                    logDebug() << "Read end of file";
                    return false;
                }

                // Check for valid origin
                if (envelope.origin() != tkm::msg::Envelope_Recipient_Server) {
                    continue;
                }

                tkm::msg::server::Message msg;
                envelope.mesg().UnpackTo(&msg);

                switch (msg.type()) {
                case tkm::msg::server::Message_Type_SetSession: {
                    IDevice::Request rq {.action = IDevice::Action::SetSession};
                    tkm::msg::server::SessionInfo sessionInfo;

                    msg.payload().UnpackTo(&sessionInfo);
                    rq.bulkData = std::make_any<tkm::msg::server::SessionInfo>(sessionInfo);

                    m_device->pushRequest(rq);
                    break;
                }
                case tkm::msg::server::Message_Type_Data: {
                    IDevice::Request rq {.action = IDevice::Action::ProcessData};
                    tkm::msg::server::Data data;

                    msg.payload().UnpackTo(&data);

                    // Set the receive timestamp
                    data.set_recvtime(time(NULL));
                    rq.bulkData = std::make_any<tkm::msg::server::Data>(data);

                    m_device->pushRequest(rq);
                    break;
                }
                case tkm::msg::server::Message_Type_Status: {
                    IDevice::Request rq {.action = IDevice::Action::Status};
                    tkm::msg::server::Status status;

                    msg.payload().UnpackTo(&status);
                    rq.bulkData = std::make_any<tkm::msg::server::Status>(status);

                    m_device->pushRequest(rq);
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
    setFinalize([this]() {
        logInfo() << "Closed connection for device: " << m_device->getDeviceData().hash();
        m_device->deleteConnection();
    });
}

void Connection::enableEvents()
{
    CollectorApp()->addEventSource(getShared());
}

Connection::~Connection()
{
    logDebug() << "Connection object destroyed for device: " << m_device->getDeviceData().hash();
    disconnect();
}

void Connection::disconnect()
{
    m_device->updateState(tkm::msg::collector::DeviceData_State_Disconnected);
    if (m_sockFd >= 0) {
        ::close(m_sockFd);
        m_sockFd = -1;
    }
}

auto Connection::connect() -> int
{
    std::string serverAddress = m_device->getDeviceData().address();
    struct hostent *server = gethostbyname(serverAddress.c_str());

    m_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &m_addr.sin_addr.s_addr, (size_t) server->h_length);
    m_addr.sin_port = htons(m_device->getDeviceData().port());

    if (::connect(m_sockFd, (struct sockaddr *) &m_addr, sizeof(struct sockaddr_in)) == -1) {
        if (errno == EINPROGRESS) {
            fd_set wfds, efds;

            FD_ZERO(&wfds);
            FD_SET(m_sockFd, &wfds);

            FD_ZERO(&efds);
            FD_SET(m_sockFd, &efds);

            // We are going to use select to wait for the socket to connect
            struct timeval tv;
            tv.tv_sec = 3;
            tv.tv_usec = 0;

            auto ret = select(m_sockFd + 1, NULL, &wfds, &efds, &tv);
            if (ret == -1) {
                logError() << "Select error on connecting" << ::strerror(errno);
                return -1;
            }
            if (ret == 0) {
                logError() << "Connection timeout";
                return -1;
            }
            if (!FD_ISSET(m_sockFd, &efds)) {
                int error = 0;
                socklen_t len = sizeof(error);

                if (getsockopt(m_sockFd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
                    logError() << "Connection failed";
                    return -1;
                }

                if (error != 0) {
                    logError() << "Connection failed. Socket error: " << ::strerror(errno);
                    return -1;
                }
            }
        } else {
            logError() << "Failed to connect to server: " << ::strerror(errno);
            return -1;
        }
    }

    // We are ready to process events
    logInfo() << "Connected to server";
    setPrepare([]() { return true; });

    return 0;
}

} // namespace tkm::collector

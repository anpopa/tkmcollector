/*-
 * Copyright (c) 2020 Alin Popa
 * All rights reserved.
 */

/*
 * @author Alin Popa <alin.popa@fxdata.ro>
 */

#include <filesystem>
#include <unistd.h>

#include "AdminClient.h"
#include "Application.h"
#include "Helpers.h"
#include "UDSServer.h"

namespace fs = std::filesystem;
using std::shared_ptr;
using std::string;

namespace cds::server
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

            // The client has 3 seconds to send the descriptor or will be disconnected
            struct timeval tv;
            tv.tv_sec = 3;
            tv.tv_usec = 0;
            setsockopt(clientFd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));

            cds::msg::client::Descriptor descriptor {};

            if (!helpers::readClientDescriptor(clientFd, descriptor)) {
                logWarn() << "Client " << clientFd << " read descriptor failed";
                close(clientFd);
                return true; // this is a client issue, process next client
            }

            switch (descriptor.type()) {
            case cds::msg::client::Descriptor::Type::Descriptor_Type_Admin: {
                logInfo() << "New AdminClient with FD: " << clientFd;
                std::shared_ptr<AdminClient> client = std::make_shared<AdminClient>(clientFd);
                client->enableEvents();
                break;
            }
            default:
                logWarn() << "Client " << clientFd << " is of unexpected type";
                close(clientFd);
                break;
            }

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
    CDSApp()->addEventSource(getShared());
}

UDSServer::~UDSServer()
{
    static_cast<void>(stop());
}

void UDSServer::start()
{
    fs::path sockPath(CDSApp()->getOptions()->getFor(server::Options::Key::RuntimeDirectory));
    sockPath /= cdsDefaults.getFor(Defaults::Default::AdminSocket);

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
        logInfo() << "Admin server listening on " << sockPath.string();
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

} // namespace cds::server

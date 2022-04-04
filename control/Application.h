#pragma once

#include <atomic>
#include <cstdlib>
#include <string>

#include "Command.h"
#include "Connection.h"
#include "Defaults.h"
#include "Dispatcher.h"
#include "Options.h"

#include "../bswinfra/source/IApplication.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/EventLoop.h"
#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/Logger.h"
#include "../bswinfra/source/PathEvent.h"
#include "../bswinfra/source/Pollable.h"
#include "../bswinfra/source/Timer.h"
#include "../bswinfra/source/UserEvent.h"

#include "Collector.pb.h"

using namespace bswi::kf;
using namespace bswi::log;
using namespace bswi::event;

namespace tkm::control
{

class Application : public bswi::app::IApplication
{
public:
    explicit Application(const std::string &name,
                         const std::string &description,
                         const std::string &configFile);

    static Application *getInstance()
    {
        return appInstance == nullptr
                   ? appInstance = new Application("TkmControl",
                                                   "TaskMonitor Collector Control Application",
                                                   tkmDefaults.getFor(Defaults::Default::ConfPath))
                   : appInstance;
    }

    void stop() final
    {
        if (m_running) {
            m_mainEventLoop->stop();
        }
    }

    void setSession(const std::string &session) { m_session = session; }
    auto getSession() -> const std::string & { return m_session; }

    auto getOptions() -> std::shared_ptr<Options> & { return m_options; }
    auto getDispatcher() -> std::shared_ptr<Dispatcher> & { return m_dispatcher; }
    auto getConnection() -> std::shared_ptr<Connection> & { return m_connection; }
    auto getCommand() -> std::shared_ptr<Command> & { return m_command; }

public:
    Application(Application const &) = delete;
    void operator=(Application const &) = delete;

private:
    std::shared_ptr<Options> m_options = nullptr;
    std::shared_ptr<Connection> m_connection = nullptr;
    std::shared_ptr<Dispatcher> m_dispatcher = nullptr;
    std::shared_ptr<Command> m_command = nullptr;
    std::string m_session {};

private:
    static Application *appInstance;
};

} // namespace tkm::control

#define ControlApp() tkm::control::Application::getInstance()

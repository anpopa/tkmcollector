#include <filesystem>
#include <stdexcept>

#include "Application.h"

using std::string;

namespace tkm::control
{

Application *Application::appInstance = nullptr;

Application::Application(const string &name, const string &description, const string &configFile)
: bswi::app::IApplication(name, description)
{
    if (Application::appInstance != nullptr) {
        throw bswi::except::SingleInstance();
    }
    appInstance = this;

    // Create Options
    m_options = std::make_shared<Options>(configFile);

    // Check runtime directory
    std::filesystem::path runDir(m_options->getFor(Options::Key::RuntimeDirectory));
    if (!std::filesystem::exists(runDir)) {
        throw std::runtime_error("Server runtime directory not available");
    }

    m_dispatcher = std::make_unique<Dispatcher>();
    m_dispatcher->enableEvents();

    m_connection = std::make_shared<Connection>();
    m_connection->enableEvents();

    m_command = std::make_unique<Command>();
    m_command->enableEvents();
}

} // namespace tkm::control

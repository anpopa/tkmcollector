#include "Options.h"
#include "Defaults.h"

using namespace std;

namespace tkm
{

Options::Options(const string &configFile)
{
    m_configFile = std::make_shared<bswi::kf::KeyFile>(configFile);
    if (m_configFile->parseFile() != 0) {
        logWarn() << "Fail to parse config file: " << configFile;
        m_configFile.reset();
    }
}

auto Options::getFor(Key key) -> string
{
    switch (key) {
    case Key::DBType:
        if (hasConfigFile()) {
            const optional<string> prop = m_configFile->getPropertyValue("database", -1, "Type");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBType));
        }
        return tkmDefaults.getFor(Defaults::Default::DBType);
    case Key::DBUserName:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("database", -1, "UserName");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBUserName));
        }
        return tkmDefaults.getFor(Defaults::Default::DBUserName);
    case Key::DBUserPassword:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("database", -1, "UserPassword");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBUserPassword));
        }
        return tkmDefaults.getFor(Defaults::Default::DBUserPassword);
    case Key::DBServerAddress:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("database", -1, "ServerAddress");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBServerAddress));
        }
        return tkmDefaults.getFor(Defaults::Default::DBServerAddress);
    case Key::DBServerPort:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("database", -1, "ServerPort");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBServerPort));
        }
        return tkmDefaults.getFor(Defaults::Default::DBServerPort);
    case Key::RuntimeDirectory:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("general", -1, "RuntimeDirectory");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::RuntimeDirectory));
        }
        return tkmDefaults.getFor(Defaults::Default::RuntimeDirectory);
    default:
        logError() << "Unknown option key";
        break;
    }

    throw std::runtime_error("Cannot provide option for key");
}

} // namespace tkm

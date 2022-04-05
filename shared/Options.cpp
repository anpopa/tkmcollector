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
    case Key::DBAddress:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("database", -1, "Address");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBAddress));
        }
        return tkmDefaults.getFor(Defaults::Default::DBAddress);
    case Key::DBPort:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("database", -1, "Port");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::DBPort));
        }
        return tkmDefaults.getFor(Defaults::Default::DBPort);
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

#pragma once

#include <any>
#include <string>

#include "Defaults.h"

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/KeyFile.h"
#include "../bswinfra/source/Logger.h"

namespace tkm {

class Options {
public:
  enum class Key {
    DBType,
    DBUserName,
    DBUserPassword,
    DBServerAddress,
    DBServerPort,
    RuntimeDirectory
  };

public:
  explicit Options(const std::string &configFile);

  auto getFor(Key key) -> std::string;
  auto hasConfigFile() -> bool { return m_configFile != nullptr; }
  auto getConfigFile() -> std::shared_ptr<bswi::kf::KeyFile> {
    return m_configFile;
  }

private:
  std::shared_ptr<bswi::kf::KeyFile> m_configFile = nullptr;
};

} // namespace tkm

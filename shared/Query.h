#pragma once

#include <map>
#include <sstream>
#include <string>

namespace tkm {

class Query {
public:
  enum class Type { SQLite3, PostgreSQL };

  auto createTables(Query::Type type) -> std::string;
  auto dropTables(Query::Type type) -> std::string;

  auto getDevices(Query::Type type) -> std::string;
  auto addDevice(Query::Type type, uint64_t id, const std::string &name,
                 const std::string &address, int32_t port, int state)
      -> std::string;
  auto remDevice(Query::Type type, uint64_t id) -> std::string;
  auto getDeviceById(Query::Type type, uint64_t id) -> std::string;
  auto getDeviceByName(Query::Type type, const std::string &name)
      -> std::string;

public:
  enum class DeviceColumn {
    Id,      // int: Primary key
    Name,    // str: Device name
    Address, // str: Device address
    Port,    // int: Device port
    State,   // int: Device state
  };
  const std::map<DeviceColumn, std::string> m_deviceColumn{
      std::make_pair(DeviceColumn::Id, "Id"),
      std::make_pair(DeviceColumn::Name, "Name"),
      std::make_pair(DeviceColumn::Address, "Address"),
      std::make_pair(DeviceColumn::Port, "Port"),
      std::make_pair(DeviceColumn::State, "State"),
  };

  const std::string m_devicesTableName = "tkmDevices";
};

static Query tkmQuery{};

} // namespace tkm

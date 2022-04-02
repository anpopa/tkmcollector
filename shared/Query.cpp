#include "Query.h"

namespace tkm
{

auto Query::createTables(Query::Type type) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        // Devices table
        out << "CREATE TABLE IF NOT EXISTS " << m_devicesTableName << " ("
            << m_deviceColumn.at(DeviceColumn::Id) << " INTEGER PRIMARY KEY, "
            << m_deviceColumn.at(DeviceColumn::Hash) << " TEXT NOT NULL, "
            << m_deviceColumn.at(DeviceColumn::Name) << " TEXT NOT NULL, "
            << m_deviceColumn.at(DeviceColumn::Address) << " TEXT NOT NULL, "
            << m_deviceColumn.at(DeviceColumn::Port) << " INTEGER NOT NULL, "
            << m_deviceColumn.at(DeviceColumn::State) << " INTEGER NOT NULL);";
    }

    return out.str();
}

auto Query::dropTables(Query::Type type) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DROP TABLE IF EXISTS " << m_devicesTableName << ";";
    }

    return out.str();
}

auto Query::getDevices(Query::Type type) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * "
            << " FROM " << m_devicesTableName << ";";
    }

    return out.str();
}

auto Query::addDevice(Query::Type type,
                      const std::string &hash,
                      const std::string &name,
                      const std::string &address,
                      int32_t port,
                      int state) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "INSERT INTO " << m_devicesTableName << " (" << m_deviceColumn.at(DeviceColumn::Hash)
            << "," << m_deviceColumn.at(DeviceColumn::Name) << ","
            << m_deviceColumn.at(DeviceColumn::Address) << ","
            << m_deviceColumn.at(DeviceColumn::Port) << ","
            << m_deviceColumn.at(DeviceColumn::State) << ") VALUES ("
            << "'" << hash << "', '" << name << "', '" << address << "', '" << port << "', '"
            << state << "');";
    }

    return out.str();
}

auto Query::remDevice(Query::Type type, const std::string &hash) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DELETE FROM " << m_devicesTableName << " WHERE "
            << m_deviceColumn.at(DeviceColumn::Hash) << " IS "
            << "'" << hash << "';";
    }

    return out.str();
}

auto Query::getDevice(Query::Type type, const std::string &hash) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * FROM " << m_devicesTableName << " WHERE "
            << m_deviceColumn.at(DeviceColumn::Hash) << " IS "
            << "'" << hash << "' LIMIT 1;";
    }

    return out.str();
}

auto Query::hasDevice(Query::Type type, const std::string &hash) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_deviceColumn.at(DeviceColumn::Id) << " FROM " << m_devicesTableName
            << " WHERE " << m_deviceColumn.at(DeviceColumn::Hash) << " IS "
            << "'" << hash << "' LIMIT 1;";
    }

    return out.str();
}

} // namespace tkm

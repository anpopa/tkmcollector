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
            << m_deviceColumn.at(DeviceColumn::Port) << " INTEGER NOT NULL);";

        // Sessions table
        out << "CREATE TABLE IF NOT EXISTS " << m_sessionsTableName << " ("
            << m_sessionColumn.at(SessionColumn::Id) << " INTEGER PRIMARY KEY, "
            << m_sessionColumn.at(SessionColumn::Name) << " TEXT NOT NULL, "
            << m_sessionColumn.at(SessionColumn::Hash) << " TEXT NOT NULL, "
            << m_sessionColumn.at(SessionColumn::StartTimestamp) << " INTEGER NOT NULL, "
            << m_sessionColumn.at(SessionColumn::EndTimestamp) << " INTEGER NOT NULL, "
            << m_sessionColumn.at(SessionColumn::Device) << " INTEGER NOT NULL, "
            << "CONSTRAINT KFDevice FOREIGN KEY(" << m_sessionColumn.at(SessionColumn::Device)
            << ") REFERENCES " << m_devicesTableName << "(" << m_deviceColumn.at(DeviceColumn::Id)
            << ") ON DELETE CASCADE);";
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
                      int32_t port) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "INSERT INTO " << m_devicesTableName << " (" << m_deviceColumn.at(DeviceColumn::Hash)
            << "," << m_deviceColumn.at(DeviceColumn::Name) << ","
            << m_deviceColumn.at(DeviceColumn::Address) << ","
            << m_deviceColumn.at(DeviceColumn::Port) << ") VALUES ("
            << "'" << hash << "', '" << name << "', '" << address << "', '" << port << "');";
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

auto Query::getSessions(Query::Type type) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * "
            << " FROM " << m_sessionsTableName << ";";
    }

    return out.str();
}

auto Query::getSessions(Query::Type type, const std::string &deviceHash) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * FROM " << m_sessionsTableName << " WHERE "
            << m_sessionColumn.at(SessionColumn::Device) << " IS "
            << "(SELECT " << m_deviceColumn.at(DeviceColumn::Id) << " FROM " << m_devicesTableName
            << " WHERE " << m_deviceColumn.at(DeviceColumn::Hash) << " IS "
            << "'" << deviceHash << "');";
    }

    return out.str();
}

auto Query::addSession(Query::Type type,
                       const std::string &hash,
                       const std::string &name,
                       uint64_t startTimestamp,
                       const std::string &deviceHash) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "INSERT INTO " << m_sessionsTableName << " ("
            << m_sessionColumn.at(SessionColumn::Hash) << ","
            << m_sessionColumn.at(SessionColumn::Name) << ","
            << m_sessionColumn.at(SessionColumn::StartTimestamp) << ","
            << m_sessionColumn.at(SessionColumn::EndTimestamp) << ","
            << m_sessionColumn.at(SessionColumn::Device) << ") VALUES ('" << hash << "', '" << name
            << "', '" << startTimestamp << "', '"
            << "0"
            << "', "
            << "(SELECT " << m_deviceColumn.at(DeviceColumn::Id) << " FROM " << m_devicesTableName
            << " WHERE " << m_deviceColumn.at(DeviceColumn::Hash) << " IS "
            << "'" << deviceHash << "'));";
    }

    return out.str();
}

auto Query::endSession(Query::Type type, const std::string &hash) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "UPDATE " << m_sessionsTableName << " SET "
            << m_sessionColumn.at(SessionColumn::EndTimestamp) << " = "
            << "'" << time(NULL) << "'"
            << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
            << "'" << hash << "';";
    }

    return out.str();
}

auto Query::remSession(Query::Type type, const std::string &hash) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DELETE FROM " << m_sessionsTableName << " WHERE "
            << m_sessionColumn.at(SessionColumn::Hash) << " IS "
            << "'" << hash << "';";
    }

    return out.str();
}

auto Query::getSession(Query::Type type, const std::string &hash) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * FROM " << m_sessionsTableName << " WHERE "
            << m_sessionColumn.at(SessionColumn::Hash) << " IS "
            << "'" << hash << "' LIMIT 1;";
    }

    return out.str();
}

auto Query::hasSession(Query::Type type, const std::string &hash) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
            << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
            << "'" << hash << "' LIMIT 1;";
    }

    return out.str();
}

} // namespace tkm

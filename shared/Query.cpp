#include "Query.h"

namespace tkm
{

auto Query::createTables(Query::Type type) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        // Users table
        out << "CREATE TABLE IF NOT EXISTS " << m_usersTableName << " ("
            << m_userColumn.at(UserColumn::Id) << " INTEGER PRIMARY KEY, "
            << m_userColumn.at(UserColumn::Name) << " TEXT NOT NULL, "
            << m_userColumn.at(UserColumn::Password) << " TEXT NOT NULL, "
            << m_userColumn.at(UserColumn::RealName) << " TEXT NOT NULL, "
            << m_userColumn.at(UserColumn::Email) << " TEXT NOT NULL, "
            << m_userColumn.at(UserColumn::Projects) << " TEXT NOT NULL);";

        // Access tokens table
        out << "CREATE TABLE IF NOT EXISTS " << m_accessTokensTableName << " ("
            << m_accessTokenColumn.at(AccessTokenColumn::Id) << " INTEGER PRIMARY KEY, "
            << m_accessTokenColumn.at(AccessTokenColumn::Name) << " TEXT NOT NULL, "
            << m_accessTokenColumn.at(AccessTokenColumn::Description) << " TEXT NOT NULL, "
            << m_accessTokenColumn.at(AccessTokenColumn::Timestamp) << " INTEGER NOT NULL, "
            << m_accessTokenColumn.at(AccessTokenColumn::Token) << " TEXT NOT NULL, "
            << m_projectColumn.at(ProjectColumn::Owner) << " INTEGER NOT NULL, "
            << "CONSTRAINT KFOwner FOREIGN KEY(" << m_accessTokenColumn.at(AccessTokenColumn::Owner)
            << ") REFERENCES " << m_usersTableName << "(" << m_userColumn.at(UserColumn::Id)
            << ") ON DELETE CASCADE);";

        // Projects table
        out << "CREATE TABLE IF NOT EXISTS " << m_projectsTableName << " ("
            << m_projectColumn.at(ProjectColumn::Id) << " INTEGER PRIMARY KEY, "
            << m_projectColumn.at(ProjectColumn::Name) << " TEXT NOT NULL, "
            << m_projectColumn.at(ProjectColumn::Description) << " TEXT NOT NULL, "
            << m_projectColumn.at(ProjectColumn::Owner) << " INTEGER NOT NULL, "
            << "CONSTRAINT KFOwner FOREIGN KEY(" << m_projectColumn.at(ProjectColumn::Owner)
            << ") REFERENCES " << m_usersTableName << "(" << m_userColumn.at(UserColumn::Id)
            << ") ON DELETE CASCADE);";

        // WorkPackages table
        out << "CREATE TABLE IF NOT EXISTS " << m_workPackagesTableName << " ("
            << m_workPackageColumn.at(WorkPackageColumn::Id) << " INTEGER PRIMARY KEY, "
            << m_workPackageColumn.at(WorkPackageColumn::Name) << " TEXT NOT NULL, "
            << m_workPackageColumn.at(WorkPackageColumn::Type) << " INTEGER NOT NULL, "
            << m_workPackageColumn.at(WorkPackageColumn::Project) << " INTEGER NOT NULL, "
            << m_workPackageColumn.at(WorkPackageColumn::Owner) << " INTEGER NOT NULL, "
            << "CONSTRAINT KFProjects FOREIGN KEY("
            << m_workPackageColumn.at(WorkPackageColumn::Project) << ") REFERENCES "
            << m_projectsTableName << "(" << m_projectColumn.at(ProjectColumn::Id)
            << ") ON DELETE CASCADE, "
            << "CONSTRAINT KFUsers FOREIGN KEY(" << m_workPackageColumn.at(WorkPackageColumn::Owner)
            << ") REFERENCES " << m_usersTableName << "(" << m_userColumn.at(UserColumn::Id)
            << ") ON DELETE CASCADE);";

        // Syslog entries table
        out << "CREATE TABLE IF NOT EXISTS " << m_slgEntriesTableName << " ("
            << m_slgEntryColumn.at(SLGEntryColumn::Id) << " INTEGER PRIMARY KEY, "
            << m_slgEntryColumn.at(SLGEntryColumn::WorkPackage) << " INTEGER NOT NULL, "
            << m_slgEntryColumn.at(SLGEntryColumn::LogIndex) << " INTEGER NOT NULL, "
            << m_slgEntryColumn.at(SLGEntryColumn::SystemTime) << " INTEGER NOT NULL, "
            << m_slgEntryColumn.at(SLGEntryColumn::Hostname) << " TEXT NOT NULL, "
            << m_slgEntryColumn.at(SLGEntryColumn::Priority) << " INTEGER NOT NULL, "
            << m_slgEntryColumn.at(SLGEntryColumn::Color) << " INTEGER NOT NULL, "
            << m_slgEntryColumn.at(SLGEntryColumn::Payload) << " TEXT NOT NULL, "
            << "CONSTRAINT KFWorkPackage FOREIGN KEY("
            << m_slgEntryColumn.at(SLGEntryColumn::WorkPackage) << ") REFERENCES "
            << m_workPackagesTableName << "(" << m_workPackageColumn.at(WorkPackageColumn::Id)
            << ") ON DELETE CASCADE);";

        // Syslog filter table
        out << "CREATE TABLE IF NOT EXISTS " << m_slgFiltersTableName << " ("
            << m_slgFilterColumn.at(SLGFilterColumn::Id) << " INTEGER PRIMARY KEY, "
            << m_slgFilterColumn.at(SLGFilterColumn::Name) << " TEXT NOT NULL, "
            << m_slgFilterColumn.at(SLGFilterColumn::Active) << " INTEGER NOT NULL, "
            << m_slgFilterColumn.at(SLGFilterColumn::Type) << " INTEGER NOT NULL, "
            << m_slgFilterColumn.at(SLGFilterColumn::Color) << " INTEGER NOT NULL, "
            << m_slgFilterColumn.at(SLGFilterColumn::Priority) << " INTEGER NOT NULL, "
            << m_slgFilterColumn.at(SLGFilterColumn::Hostname) << " TEXT NOT NULL, "
            << m_slgFilterColumn.at(SLGFilterColumn::Payload) << " TEXT NOT NULL, "
            << m_slgFilterColumn.at(SLGFilterColumn::Owner) << " INTEGER NOT NULL, "
            << "CONSTRAINT KFOwner FOREIGN KEY(" << m_slgFilterColumn.at(SLGFilterColumn::Owner)
            << ") REFERENCES " << m_usersTableName << "(" << m_userColumn.at(UserColumn::Id)
            << ") ON DELETE CASCADE);";

        // DLT entries table
        out << "CREATE TABLE IF NOT EXISTS " << m_dltEntriesTableName << " ("
            << m_dltEntryColumn.at(DLTEntryColumn::Id) << " INTEGER PRIMARY KEY, "
            << m_dltEntryColumn.at(DLTEntryColumn::WorkPackage) << " INTEGER NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::LogIndex) << " INTEGER NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::SystemTime) << " INTEGER NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::MonotonicTime) << " INTEGER NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::LocalCount) << " INTEGER NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::ArgsNo) << " INTEGER NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::EcuID) << " TEXT NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::AppID) << " TEXT NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::CtxID) << " TEXT NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::Type) << " INTEGER NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::SubType) << " INTEGER NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::Color) << " INTEGER NOT NULL, "
            << m_dltEntryColumn.at(DLTEntryColumn::Payload) << " TEXT NOT NULL, "
            << "CONSTRAINT KFWorkPackage FOREIGN KEY("
            << m_dltEntryColumn.at(DLTEntryColumn::WorkPackage) << ") REFERENCES "
            << m_workPackagesTableName << "(" << m_workPackageColumn.at(WorkPackageColumn::Id)
            << ") ON DELETE CASCADE);";

        // DLT filter table
        out << "CREATE TABLE IF NOT EXISTS " << m_dltFiltersTableName << " ("
            << m_dltFilterColumn.at(DLTFilterColumn::Id) << " INTEGER PRIMARY KEY, "
            << m_dltFilterColumn.at(DLTFilterColumn::Name) << " TEXT NOT NULL, "
            << m_dltFilterColumn.at(DLTFilterColumn::Active) << " INTEGER NOT NULL, "
            << m_dltFilterColumn.at(DLTFilterColumn::Type) << " INTEGER NOT NULL, "
            << m_dltFilterColumn.at(DLTFilterColumn::Color) << " INTEGER NOT NULL, "
            << m_dltFilterColumn.at(DLTFilterColumn::MsgType) << " INTEGER NOT NULL, "
            << m_dltFilterColumn.at(DLTFilterColumn::MsgSubType) << " INTEGER NOT NULL, "
            << m_dltFilterColumn.at(DLTFilterColumn::EcuID) << " TEXT NOT NULL, "
            << m_dltFilterColumn.at(DLTFilterColumn::AppID) << " TEXT NOT NULL, "
            << m_dltFilterColumn.at(DLTFilterColumn::CtxID) << " TEXT NOT NULL, "
            << m_dltFilterColumn.at(DLTFilterColumn::Payload) << " TEXT NOT NULL, "
            << m_dltFilterColumn.at(DLTFilterColumn::Owner) << " INTEGER NOT NULL, "
            << "CONSTRAINT KFOwner FOREIGN KEY(" << m_dltFilterColumn.at(DLTFilterColumn::Owner)
            << ") REFERENCES " << m_usersTableName << "(" << m_userColumn.at(UserColumn::Id)
            << ") ON DELETE CASCADE);";
    }

    return out.str();
}

auto Query::dropTables(Query::Type type) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DROP TABLE IF EXISTS " << m_usersTableName << ";";
        out << "DROP TABLE IF EXISTS " << m_accessTokensTableName << ";";
        out << "DROP TABLE IF EXISTS " << m_projectsTableName << ";";
        out << "DROP TABLE IF EXISTS " << m_workPackagesTableName << ";";
        out << "DROP TABLE IF EXISTS " << m_slgEntriesTableName << ";";
        out << "DROP TABLE IF EXISTS " << m_slgFiltersTableName << ";";
        out << "DROP TABLE IF EXISTS " << m_dltEntriesTableName << ";";
        out << "DROP TABLE IF EXISTS " << m_dltFiltersTableName << ";";
    }

    return out.str();
}

auto Query::getUsers(Query::Type type) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * "
            << " FROM " << m_usersTableName << ";";
    }

    return out.str();
}

auto Query::addUser(Query::Type type,
                    const std::string &name,
                    const std::string &password,
                    const std::string &realname,
                    const std::string &email,
                    const std::string &projects) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "INSERT INTO " << m_usersTableName << " (" << m_userColumn.at(UserColumn::Name)
            << "," << m_userColumn.at(UserColumn::Password) << ","
            << m_userColumn.at(UserColumn::RealName) << "," << m_userColumn.at(UserColumn::Email)
            << "," << m_userColumn.at(UserColumn::Projects) << ") VALUES ("
            << "'" << name << "', '" << password << "', '" << realname << "', '" << email << "', '"
            << projects << "');";
    }

    return out.str();
}

auto Query::authUser(Query::Type type,
                     const std::string &name,
                     const std::string &password,
                     bool isToken) -> std::string
{
    std::stringstream out;

    /*
     * SELECT User::Id FROM UsersTable INNER JOIN TokensTable ON TokensTable.Token::Owner =
     * UsersTable.User::Id WHERE (User::Name IS $name) AND ((User::Password IS $password) OR
     * Token::Token IS $password)) LIMIT 1;
     */
    if (type == Query::Type::SQLite3) {
        if (!isToken) {
            out << "SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
                << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
                << "'" << name << "' AND " << m_userColumn.at(UserColumn::Password) << " IS "
                << "'" << password << "' LIMIT 1;";
        } else {
            out << "SELECT " << m_accessTokenColumn.at(AccessTokenColumn::Owner) << " FROM "
                << m_accessTokensTableName << " WHERE "
                << m_accessTokenColumn.at(AccessTokenColumn::Token) << " IS "
                << "'" << password << "' AND " << m_accessTokenColumn.at(AccessTokenColumn::Owner)
                << " IS "
                << "(SELECT " << m_usersTableName << "." << m_userColumn.at(UserColumn::Id)
                << " FROM " << m_usersTableName << " WHERE " << m_usersTableName << "."
                << m_userColumn.at(UserColumn::Name) << " IS "
                << "'" << name << "') LIMIT 1;";
        }
    }

    return out.str();
}

auto Query::getAccessTokens(Query::Type type, const std::string &owner) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * FROM " << m_accessTokensTableName << " WHERE "
            << m_accessTokenColumn.at(AccessTokenColumn::Owner) << " IS "
            << "(SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS '" << owner << "');";
    }

    return out.str();
}

auto Query::addAccessToken(Query::Type type,
                           const std::string &name,
                           const std::string &description,
                           uint64_t timestamp,
                           const std::string &token,
                           const std::string &owner) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "INSERT INTO " << m_accessTokensTableName << " ("
            << m_accessTokenColumn.at(AccessTokenColumn::Name) << ","
            << m_accessTokenColumn.at(AccessTokenColumn::Description) << ","
            << m_accessTokenColumn.at(AccessTokenColumn::Timestamp) << ","
            << m_accessTokenColumn.at(AccessTokenColumn::Token) << ","
            << m_accessTokenColumn.at(AccessTokenColumn::Owner) << ") VALUES ('" << name << "', '"
            << description << "', '" << timestamp << "', '" << token << "', "
            << "(SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << owner << "'));";
    }

    return out.str();
}

auto Query::remAccessToken(Query::Type type, const std::string &name, const std::string &owner)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DELETE FROM " << m_accessTokensTableName << " WHERE "
            << m_accessTokenColumn.at(AccessTokenColumn::Token) << " IS "
            << "'" << name << "' AND " << m_accessTokenColumn.at(AccessTokenColumn::Owner) << " IS "
            << "(SELECT " << m_usersTableName << "." << m_userColumn.at(UserColumn::Id) << " FROM "
            << m_usersTableName << " WHERE " << m_usersTableName << "."
            << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << owner << "');";
    }

    return out.str();
}

auto Query::addDltEntry(Query::Type type,
                        int workPackage,
                        int msgColor,
                        int msgType,
                        int msgSubType,
                        uint64_t index,
                        uint64_t systemTime,
                        uint64_t monotonicTime,
                        uint64_t localCount,
                        uint64_t argsCount,
                        const std::string &ecuID,
                        const std::string &appID,
                        const std::string &ctxID,
                        const std::string &payload) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "INSERT INTO " << m_dltEntriesTableName << " ("
            << m_dltEntryColumn.at(DLTEntryColumn::WorkPackage) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::LogIndex) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::SystemTime) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::MonotonicTime) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::LocalCount) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::ArgsNo) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::EcuID) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::AppID) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::CtxID) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::Type) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::SubType) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::Color) << ","
            << m_dltEntryColumn.at(DLTEntryColumn::Payload) << ") VALUES (" << workPackage << ", "
            << index << ", " << systemTime << ", " << monotonicTime << ", " << localCount << ", "
            << argsCount << ", '" << ecuID << "', '" << appID << "', '" << ctxID << "', " << msgType
            << ", " << msgSubType << ", " << msgColor << ", '" << payload << "');";
    }

    return out.str();
}

auto Query::remDltEntry(Query::Type type, int workPackage, uint64_t index) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DELETE FROM " << m_dltEntriesTableName << " WHERE "
            << m_dltEntryColumn.at(DLTEntryColumn::WorkPackage) << " IS " << workPackage << " AND "
            << m_dltEntryColumn.at(DLTEntryColumn::LogIndex) << " IS " << index << ");";
    }

    return out.str();
}

auto Query::setDltEntryColor(Query::Type type, int workPackage, uint64_t index, int color)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "UPDATE " << m_dltEntriesTableName << " SET "
            << m_dltEntryColumn.at(DLTEntryColumn::Color) << " = " << color << " WHERE (("
            << m_dltEntryColumn.at(DLTEntryColumn::WorkPackage) << " IS " << workPackage
            << ") AND (" << m_dltEntryColumn.at(DLTEntryColumn::LogIndex) << " IS " << index
            << "));";
    }

    return out.str();
}

auto Query::getDltEntries(Query::Type type, int workPackage, uint64_t indexStart, uint64_t indexEnd)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * FROM " << m_dltEntriesTableName << " WHERE "
            << m_dltEntryColumn.at(DLTEntryColumn::WorkPackage) << " IS " << workPackage << " AND ("
            << m_dltEntryColumn.at(DLTEntryColumn::LogIndex) << " >= " << indexStart << " AND "
            << m_dltEntryColumn.at(DLTEntryColumn::LogIndex) << " < " << indexEnd << "));";
    }

    return out.str();
}

auto Query::addDltFilter(Query::Type type,
                         const std::string &name,
                         const std::string &owner,
                         bool active,
                         int fType,
                         int fColor,
                         int fMsgType,
                         int fMsgSubType,
                         const std::string &ecuID,
                         const std::string &appID,
                         const std::string &ctxID,
                         const std::string &payload) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "INSERT INTO " << m_dltFiltersTableName << " ("
            << m_dltFilterColumn.at(DLTFilterColumn::Name) << ","
            << m_dltFilterColumn.at(DLTFilterColumn::Active) << ","
            << m_dltFilterColumn.at(DLTFilterColumn::Type) << ","
            << m_dltFilterColumn.at(DLTFilterColumn::Color) << ","
            << m_dltFilterColumn.at(DLTFilterColumn::MsgType) << ","
            << m_dltFilterColumn.at(DLTFilterColumn::MsgSubType) << ","
            << m_dltFilterColumn.at(DLTFilterColumn::EcuID) << ","
            << m_dltFilterColumn.at(DLTFilterColumn::AppID) << ","
            << m_dltFilterColumn.at(DLTFilterColumn::CtxID) << ","
            << m_dltFilterColumn.at(DLTFilterColumn::Payload) << ","
            << m_dltFilterColumn.at(DLTFilterColumn::Owner) << ") VALUES ('" << name << "', "
            << static_cast<int>(active) << ", " << fType << ", " << fColor << ", " << fMsgType
            << ", " << fMsgSubType << ", '" << ecuID << "', '" << appID << "', '" << ctxID << "', '"
            << payload << "', "
            << "(SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << owner << "'));";
    }

    return out.str();
}

auto Query::remDltFilter(Query::Type type, const std::string &name, const std::string &owner)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DELETE FROM " << m_dltFiltersTableName << " WHERE "
            << m_dltFilterColumn.at(DLTFilterColumn::Name) << " IS "
            << "'" << name << "' AND " << m_dltFilterColumn.at(DLTFilterColumn::Owner) << " IS "
            << "(SELECT " << m_usersTableName << "." << m_userColumn.at(UserColumn::Id) << " FROM "
            << m_usersTableName << " WHERE " << m_usersTableName << "."
            << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << owner << "');";
    }

    return out.str();
}

auto Query::getDltFilters(Query::Type type, const std::string &owner) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * FROM " << m_dltFiltersTableName << " WHERE "
            << m_dltFilterColumn.at(DLTFilterColumn::Owner) << " IS "
            << "(SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS '" << owner << "');";
    }

    return out.str();
}

auto Query::getDltFilterId(Query::Type type, const std::string &name, const std::string &owner)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_dltFilterColumn.at(DLTFilterColumn::Id) << " FROM "
            << m_dltFiltersTableName << " WHERE " << m_dltFilterColumn.at(DLTFilterColumn::Name)
            << " IS '" << name << "' AND " << m_dltFilterColumn.at(DLTFilterColumn::Owner) << " IS "
            << "(SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS '" << owner << "');";
    }

    return out.str();
}

auto Query::setDltFilterActive(Query::Type type,
                               const std::string &name,
                               const std::string &owner,
                               bool active) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "UPDATE " << m_dltFiltersTableName << " SET "
            << m_dltFilterColumn.at(DLTFilterColumn::Active) << " = " << static_cast<int>(active)
            << " WHERE " << m_dltFilterColumn.at(DLTFilterColumn::Owner) << " IS "
            << "(SELECT " << m_usersTableName << "." << m_userColumn.at(UserColumn::Id) << " FROM "
            << m_usersTableName << " WHERE " << m_usersTableName << "."
            << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << owner << "');";
    }

    return out.str();
}

auto Query::addSyslogEntry(Query::Type type,
                           int workPackage,
                           int color,
                           int priority,
                           uint64_t index,
                           uint64_t systemTime,
                           const std::string &hostname,
                           const std::string &payload) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "INSERT INTO " << m_slgEntriesTableName << " ("
            << m_slgEntryColumn.at(SLGEntryColumn::WorkPackage) << ","
            << m_slgEntryColumn.at(SLGEntryColumn::LogIndex) << ","
            << m_slgEntryColumn.at(SLGEntryColumn::SystemTime) << ","
            << m_slgEntryColumn.at(SLGEntryColumn::Hostname) << ","
            << m_slgEntryColumn.at(SLGEntryColumn::Priority) << ","
            << m_slgEntryColumn.at(SLGEntryColumn::Color) << ","
            << m_slgEntryColumn.at(SLGEntryColumn::Payload) << ") VALUES (" << workPackage << ", "
            << index << ", " << systemTime << ", '" << hostname << "', " << priority << ", "
            << color << ", '" << payload << "');";
    }

    return out.str();
}

auto Query::remSyslogEntry(Query::Type type, int workPackage, uint64_t index) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DELETE FROM " << m_slgEntriesTableName << " WHERE "
            << m_slgEntryColumn.at(SLGEntryColumn::WorkPackage) << " IS " << workPackage << " AND "
            << m_slgEntryColumn.at(SLGEntryColumn::LogIndex) << " IS " << index << ");";
    }

    return out.str();
}

auto Query::setSyslogEntryColor(Query::Type type, int workPackage, uint64_t index, int color)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "UPDATE " << m_slgEntriesTableName << " SET "
            << m_slgEntryColumn.at(SLGEntryColumn::Color) << " = " << color << " WHERE (("
            << m_slgEntryColumn.at(SLGEntryColumn::WorkPackage) << " IS " << workPackage
            << ") AND (" << m_slgEntryColumn.at(SLGEntryColumn::LogIndex) << " IS " << index
            << "));";
    }

    return out.str();
}

auto Query::getSyslogEntries(Query::Type type,
                             int workPackage,
                             uint64_t indexStart,
                             uint64_t indexEnd) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * FROM " << m_slgEntriesTableName << " WHERE "
            << m_slgEntryColumn.at(SLGEntryColumn::WorkPackage) << " IS " << workPackage << " AND ("
            << m_slgEntryColumn.at(SLGEntryColumn::LogIndex) << " >= " << indexStart << " AND "
            << m_slgEntryColumn.at(SLGEntryColumn::LogIndex) << " < " << indexEnd << "));";
    }

    return out.str();
}

auto Query::addSyslogFilter(Query::Type type,
                            const std::string &name,
                            const std::string &owner,
                            bool active,
                            int fType,
                            int fColor,
                            int fPriority,
                            const std::string &hostname,
                            const std::string &payload) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "INSERT INTO " << m_slgFiltersTableName << " ("
            << m_slgFilterColumn.at(SLGFilterColumn::Name) << ","
            << m_slgFilterColumn.at(SLGFilterColumn::Active) << ","
            << m_slgFilterColumn.at(SLGFilterColumn::Type) << ","
            << m_slgFilterColumn.at(SLGFilterColumn::Color) << ","
            << m_slgFilterColumn.at(SLGFilterColumn::Priority) << ","
            << m_slgFilterColumn.at(SLGFilterColumn::Hostname) << ","
            << m_slgFilterColumn.at(SLGFilterColumn::Payload) << ","
            << m_slgFilterColumn.at(SLGFilterColumn::Owner) << ") VALUES ('" << name << "', "
            << static_cast<int>(active) << ", " << fType << ", " << fColor << ", " << fPriority
            << ", '" << hostname << "', '" << payload << "', "
            << "(SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << owner << "'));";
    }

    return out.str();
}

auto Query::remSyslogFilter(Query::Type type, const std::string &name, const std::string &owner)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DELETE FROM " << m_slgFiltersTableName << " WHERE "
            << m_slgFilterColumn.at(SLGFilterColumn::Name) << " IS "
            << "'" << name << "' AND " << m_slgFilterColumn.at(SLGFilterColumn::Owner) << " IS "
            << "(SELECT " << m_usersTableName << "." << m_userColumn.at(UserColumn::Id) << " FROM "
            << m_usersTableName << " WHERE " << m_usersTableName << "."
            << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << owner << "');";
    }

    return out.str();
}

auto Query::getSyslogFilters(Query::Type type, const std::string &owner) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * FROM " << m_slgFiltersTableName << " WHERE "
            << m_slgFilterColumn.at(SLGFilterColumn::Owner) << " IS "
            << "(SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS '" << owner << "');";
    }

    return out.str();
}

auto Query::getSyslogFilterId(Query::Type type, const std::string &name, const std::string &owner)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_slgFilterColumn.at(SLGFilterColumn::Id) << " FROM "
            << m_slgFiltersTableName << " WHERE " << m_slgFilterColumn.at(SLGFilterColumn::Name)
            << " IS '" << name << "' AND " << m_slgFilterColumn.at(SLGFilterColumn::Owner) << " IS "
            << "(SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS '" << owner << "');";
    }

    return out.str();
}

auto Query::setSyslogFilterActive(Query::Type type,
                                  const std::string &name,
                                  const std::string &owner,
                                  bool active) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "UPDATE " << m_slgFiltersTableName << " SET "
            << m_slgFilterColumn.at(SLGFilterColumn::Active) << " = " << static_cast<int>(active)
            << " WHERE " << m_slgFilterColumn.at(SLGFilterColumn::Owner) << " IS "
            << "(SELECT " << m_usersTableName << "." << m_userColumn.at(UserColumn::Id) << " FROM "
            << m_usersTableName << " WHERE " << m_usersTableName << "."
            << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << owner << "');";
    }

    return out.str();
}

auto Query::getProjects(Query::Type type) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * "
            << " FROM " << m_projectsTableName << ";";
    }

    return out.str();
}

auto Query::addProject(Query::Type type,
                       const std::string &name,
                       const std::string &description,
                       const std::string &owner) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "INSERT INTO " << m_projectsTableName << " ("
            << m_projectColumn.at(ProjectColumn::Name) << ","
            << m_projectColumn.at(ProjectColumn::Description) << ","
            << m_projectColumn.at(ProjectColumn::Owner) << ") VALUES ('" << name << "', '"
            << description << "', "
            << "(SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << owner << "'));";
    }

    return out.str();
}

auto Query::addWorkPackage(Query::Type type,
                           const std::string &name,
                           int64_t wstype,
                           const std::string &project,
                           const std::string &owner) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "INSERT INTO " << m_workPackagesTableName << " ("
            << m_workPackageColumn.at(WorkPackageColumn::Name) << ","
            << m_workPackageColumn.at(WorkPackageColumn::Type) << ","
            << m_workPackageColumn.at(WorkPackageColumn::Project) << ","
            << m_workPackageColumn.at(WorkPackageColumn::Owner) << ") VALUES ('" << name << "', "
            << wstype << ", "
            << "(SELECT " << m_projectColumn.at(ProjectColumn::Id) << " FROM "
            << m_projectsTableName << " WHERE " << m_projectColumn.at(ProjectColumn::Name) << " IS "
            << "'" << project << "'), "
            << "(SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << owner << "'));";
    }

    return out.str();
}

auto Query::remUser(Query::Type type, const std::string &name) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DELETE FROM " << m_usersTableName << " WHERE " << m_userColumn.at(UserColumn::Name)
            << " IS "
            << "'" << name << "';";
    }

    return out.str();
}

auto Query::getUserId(Query::Type type, const std::string &name) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_userColumn.at(UserColumn::Id) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << name << "' LIMIT 1;";
    }

    return out.str();
}

auto Query::getUserPassword(Query::Type type, const std::string &name) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_userColumn.at(UserColumn::Password) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << name << "' LIMIT 1;";
    }

    return out.str();
}

auto Query::getUserRealName(Query::Type type, const std::string &name) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_userColumn.at(UserColumn::RealName) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << name << "' LIMIT 1;";
    }

    return out.str();
}

auto Query::getUserEmail(Query::Type type, const std::string &name) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_userColumn.at(UserColumn::Email) << " FROM " << m_usersTableName
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << name << "' LIMIT 1;";
    }

    return out.str();
}

auto Query::updUserPassword(Query::Type type, const std::string &name, const std::string &password)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "UPDATE " << m_usersTableName << " SET " << m_userColumn.at(UserColumn::Password)
            << " = "
            << "'" << password << "'"
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << name << "';";
    }

    return out.str();
}

auto Query::updUserRealName(Query::Type type, const std::string &name, const std::string &realname)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "UPDATE " << m_usersTableName << " SET " << m_userColumn.at(UserColumn::RealName)
            << " = "
            << "'" << realname << "'"
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << name << "';";
    }

    return out.str();
}

auto Query::updUserEmail(Query::Type type, const std::string &name, const std::string &email)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "UPDATE " << m_usersTableName << " SET " << m_userColumn.at(UserColumn::Email)
            << " = "
            << "'" << email << "'"
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << name << "';";
    }

    return out.str();
}

auto Query::updUserProjects(Query::Type type, const std::string &name, const std::string &projects)
    -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "UPDATE " << m_usersTableName << " SET " << m_userColumn.at(UserColumn::Projects)
            << " = "
            << "'" << projects << "'"
            << " WHERE " << m_userColumn.at(UserColumn::Name) << " IS "
            << "'" << name << "';";
    }

    return out.str();
}

auto Query::getProjectDescription(Query::Type type, const std::string &name) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_projectColumn.at(ProjectColumn::Description) << " FROM "
            << m_projectsTableName << " WHERE " << m_projectColumn.at(ProjectColumn::Name) << " IS "
            << "'" << name << "' LIMIT 1;";
    }

    return out.str();
}

auto Query::getProjectId(Query::Type type, const std::string &name) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_projectColumn.at(ProjectColumn::Id) << " FROM " << m_projectsTableName
            << " WHERE " << m_projectColumn.at(ProjectColumn::Name) << " IS "
            << "'" << name << "' LIMIT 1;";
    }

    return out.str();
}

auto Query::remProject(Query::Type type, const std::string &name) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DELETE FROM " << m_projectsTableName << " WHERE "
            << m_projectColumn.at(ProjectColumn::Name) << " IS "
            << "'" << name << "';";
    }

    return out.str();
}

auto Query::updUpdateProjectDescription(Query::Type type,
                                        const std::string &name,
                                        const std::string &desc) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "UPDATE " << m_projectsTableName << " SET "
            << m_projectColumn.at(ProjectColumn::Description) << " = " << desc << " WHERE "
            << m_projectColumn.at(ProjectColumn::Name) << " IS "
            << "'" << name << "';";
    }

    return out.str();
}

auto Query::remWorkPackage(Query::Type type, const std::string &name) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "DELETE FROM " << m_workPackagesTableName << " WHERE "
            << m_workPackageColumn.at(WorkPackageColumn::Name) << " IS "
            << "'" << name << "';";
    }

    return out.str();
}

auto Query::getWorkPackages(Query::Type type) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT * "
            << " FROM " << m_workPackagesTableName << ";";
    }

    return out.str();
}

auto Query::getWorkPackageId(Query::Type type, const std::string &name) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_workPackageColumn.at(WorkPackageColumn::Id) << " FROM "
            << m_workPackagesTableName << " WHERE "
            << m_workPackageColumn.at(WorkPackageColumn::Name) << " IS "
            << "'" << name << "' LIMIT 1;";
    }

    return out.str();
}

auto Query::getWPProjectId(Query::Type type, const std::string &wpname) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_workPackageColumn.at(WorkPackageColumn::Project) << " FROM "
            << m_workPackagesTableName << " WHERE "
            << m_workPackageColumn.at(WorkPackageColumn::Name) << " IS "
            << "'" << wpname << "' LIMIT 1;";
    }

    return out.str();
}

auto Query::getWPOwnerId(Query::Type type, const std::string &wpname) -> std::string
{
    std::stringstream out;

    if (type == Query::Type::SQLite3) {
        out << "SELECT " << m_workPackageColumn.at(WorkPackageColumn::Owner) << " FROM "
            << m_workPackagesTableName << " WHERE "
            << m_workPackageColumn.at(WorkPackageColumn::Name) << " IS "
            << "'" << wpname << "' LIMIT 1;";
    }

    return out.str();
}

} // namespace tkm

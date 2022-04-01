#pragma once

#include <map>
#include <sstream>
#include <string>

namespace tkm
{

class Query
{
public:
    enum class Type { SQLite3, PostgreSQL };

    auto createTables(Query::Type type) -> std::string;
    auto dropTables(Query::Type type) -> std::string;

    auto getUsers(Query::Type type) -> std::string;
    auto addUser(Query::Type type,
                 const std::string &name,
                 const std::string &password,
                 const std::string &realname,
                 const std::string &email,
                 const std::string &projects) -> std::string;
    auto authUser(Query::Type type,
                  const std::string &name,
                  const std::string &password,
                  bool isToken = false) -> std::string;
    auto remUser(Query::Type type, const std::string &name) -> std::string;
    auto getUserId(Query::Type type, const std::string &name) -> std::string;
    auto getUserPassword(Query::Type type, const std::string &name) -> std::string;
    auto getUserRealName(Query::Type type, const std::string &name) -> std::string;
    auto getUserEmail(Query::Type type, const std::string &name) -> std::string;
    auto updUserPassword(Query::Type type, const std::string &name, const std::string &password)
        -> std::string;
    auto updUserRealName(Query::Type type, const std::string &name, const std::string &realname)
        -> std::string;
    auto updUserEmail(Query::Type type, const std::string &name, const std::string &email)
        -> std::string;
    auto updUserProjects(Query::Type type, const std::string &name, const std::string &email)
        -> std::string;

    auto getAccessTokens(Query::Type type, const std::string &owner) -> std::string;
    auto addAccessToken(Query::Type type,
                        const std::string &name,
                        const std::string &description,
                        uint64_t timestamp,
                        const std::string &token,
                        const std::string &owner) -> std::string;
    auto remAccessToken(Query::Type type, const std::string &name, const std::string &owner)
        -> std::string;

    auto addDltEntry(Query::Type type,
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
                     const std::string &payload) -> std::string;
    auto remDltEntry(Query::Type type, int workPackage, uint64_t index) -> std::string;
    auto setDltEntryColor(Query::Type type, int workPackage, uint64_t index, int color)
        -> std::string;
    auto getDltEntries(Query::Type type, int workPackage, uint64_t indexStart, uint64_t indexEnd)
        -> std::string;

    auto addDltFilter(Query::Type type,
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
                      const std::string &payload) -> std::string;
    auto remDltFilter(Query::Type type, const std::string &name, const std::string &owner)
        -> std::string;
    auto setDltFilterActive(Query::Type type,
                            const std::string &name,
                            const std::string &owner,
                            bool active) -> std::string;
    auto getDltFilters(Query::Type type, const std::string &owner) -> std::string;
    auto getDltFilterId(Query::Type type, const std::string &name, const std::string &owner)
        -> std::string;

    auto addSyslogEntry(Query::Type type,
                        int workPackage,
                        int color,
                        int priority,
                        uint64_t index,
                        uint64_t systemTime,
                        const std::string &hostname,
                        const std::string &payload) -> std::string;
    auto remSyslogEntry(Query::Type type, int workPackage, uint64_t index) -> std::string;
    auto setSyslogEntryColor(Query::Type type, int workPackage, uint64_t index, int color)
        -> std::string;
    auto getSyslogEntries(Query::Type type, int workPackage, uint64_t indexStart, uint64_t indexEnd)
        -> std::string;

    auto addSyslogFilter(Query::Type type,
                         const std::string &name,
                         const std::string &owner,
                         bool active,
                         int fType,
                         int fColor,
                         int fPriority,
                         const std::string &hostname,
                         const std::string &payload) -> std::string;
    auto remSyslogFilter(Query::Type type, const std::string &name, const std::string &owner)
        -> std::string;
    auto setSyslogFilterActive(Query::Type type,
                               const std::string &name,
                               const std::string &owner,
                               bool active) -> std::string;
    auto getSyslogFilters(Query::Type type, const std::string &owner) -> std::string;
    auto getSyslogFilterId(Query::Type type, const std::string &name, const std::string &owner)
        -> std::string;

    auto getProjects(Query::Type type) -> std::string;
    auto addProject(Query::Type type,
                    const std::string &name,
                    const std::string &description,
                    const std::string &owner) -> std::string;
    auto remProject(Query::Type type, const std::string &name) -> std::string;
    auto getProjectId(Query::Type type, const std::string &name) -> std::string;
    auto getProjectDescription(Query::Type type, const std::string &name) -> std::string;
    auto updUpdateProjectDescription(Query::Type type,
                                     const std::string &name,
                                     const std::string &desc) -> std::string;

    auto remWorkPackage(Query::Type type, const std::string &name) -> std::string;
    auto getWorkPackages(Query::Type type) -> std::string;
    auto getWorkPackageId(Query::Type type, const std::string &name) -> std::string;
    auto addWorkPackage(Query::Type type,
                        const std::string &name,
                        int64_t wptype,
                        const std::string &project,
                        const std::string &owner) -> std::string;
    auto getWPProjectId(Query::Type type, const std::string &wpname) -> std::string;
    auto getWPOwnerId(Query::Type type, const std::string &wpname) -> std::string;

public:
    enum class UserColumn {
        Id,       // int: Primary key
        Name,     // str: Unique user name
        Password, // str: Unique user name
        RealName, // str: Real user name
        Email,    // str: User email address
        Projects  // str: Comma separated list of projects
    };
    const std::map<UserColumn, std::string> m_userColumn {
        std::make_pair(UserColumn::Id, "Id"),
        std::make_pair(UserColumn::Name, "Name"),
        std::make_pair(UserColumn::Password, "Password"),
        std::make_pair(UserColumn::RealName, "RealName"),
        std::make_pair(UserColumn::Email, "Email"),
        std::make_pair(UserColumn::Projects, "Projects"),
    };

    enum class AccessTokenColumn {
        Id,          // int: Primary key
        Name,        // str: Unique per user token name
        Description, // str: Token description
        Timestamp,   // int: Token creation timestamp
        Owner,       // str: Token owner
        Token        // str: Hashed token
    };
    const std::map<AccessTokenColumn, std::string> m_accessTokenColumn {
        std::make_pair(AccessTokenColumn::Id, "Id"),
        std::make_pair(AccessTokenColumn::Name, "Name"),
        std::make_pair(AccessTokenColumn::Description, "Description"),
        std::make_pair(AccessTokenColumn::Timestamp, "Timestamp"),
        std::make_pair(AccessTokenColumn::Owner, "Owner"),
        std::make_pair(AccessTokenColumn::Token, "Token"),
    };

    enum class ProjectColumn {
        Id,         // int: Primary key
        Name,       // str: Unique project name
        Owner,      // str: Project owner primary key
        Description // str: Project description
    };
    const std::map<ProjectColumn, std::string> m_projectColumn {
        std::make_pair(ProjectColumn::Id, "Id"),
        std::make_pair(ProjectColumn::Name, "Name"),
        std::make_pair(ProjectColumn::Owner, "Owner"),
        std::make_pair(ProjectColumn::Description, "Description"),
    };

    enum class WorkPackageColumn {
        Id,      // int: Primary key
        Name,    // str: Unique WP name
        Type,    // int: Work package type (Syslog=0, DLT=1)
        Project, // int: Project primary key
        Owner    // int: Owner primary key
    };
    const std::map<WorkPackageColumn, std::string> m_workPackageColumn {
        std::make_pair(WorkPackageColumn::Id, "Id"),
        std::make_pair(WorkPackageColumn::Name, "Name"),
        std::make_pair(WorkPackageColumn::Type, "Type"),
        std::make_pair(WorkPackageColumn::Project, "Project"),
        std::make_pair(WorkPackageColumn::Owner, "Owner"),
    };

    enum class SLGEntryColumn {
        Id,          // int: Primary key
        WorkPackage, // int: WorkPackage primary key
        LogIndex,    // uint: Log entry index
        SystemTime,  // uint: Log entry system time
        Hostname,    // str: Log entry hostname
        Priority,    // int: Log entry priority (Verbose=0,Debug=1,Info=2,Warn=3,Error=4,Fatal=5)
        Color, // int: Log color (Default=0,Red=1,Yellow=2,Cyan=3,Mangenta=4,Brown=5,Blue=6,Green=7)
        Payload // str: Log entry payload
    };
    const std::map<SLGEntryColumn, std::string> m_slgEntryColumn {
        std::make_pair(SLGEntryColumn::Id, "Id"),
        std::make_pair(SLGEntryColumn::WorkPackage, "WorkPackage"),
        std::make_pair(SLGEntryColumn::LogIndex, "LogIndex"),
        std::make_pair(SLGEntryColumn::SystemTime, "SystemTime"),
        std::make_pair(SLGEntryColumn::Hostname, "Hostname"),
        std::make_pair(SLGEntryColumn::Priority, "Priority"),
        std::make_pair(SLGEntryColumn::Color, "Color"),
        std::make_pair(SLGEntryColumn::Payload, "Payload"),
    };

    enum class DLTEntryColumn {
        Id,            // int: Primary key
        WorkPackage,   // int: WorkPackage primary key
        LogIndex,      // uint: Log entry index
        SystemTime,    // uint: Log entry system time
        MonotonicTime, // uint: Log entry monotonic time
        LocalCount,    // uint: Log entry local count
        ArgsNo,        // uint: Log entry number of arguments
        EcuID,         // str: Log entry ECU ID
        AppID,         // str: Log entry app ID
        CtxID,         // str: Log entry context ID
        Type,          // int: Log entry type (AppTrace=1,NetworkTrace=2,Control=3)
        SubType, // int: Log entry type (Fatal=0,Error=1,Warn=2,Info=3,Debug=4,Verbose=5,Variable=6,
                 // FuncIn=7,FuncOut=8,State=9,Vfb=10,Ipc=11,CAN=12,MOST=13,Request=14,Response=15,
                 // Time= 16)
        Color, // int: Log color (Default=0,Red=1,Yellow=2,Cyan=3,Mangenta=4,Brown=5,Blue=6,Green=7)
        Payload // str: Log entry payload
    };
    const std::map<DLTEntryColumn, std::string> m_dltEntryColumn {
        std::make_pair(DLTEntryColumn::Id, "Id"),
        std::make_pair(DLTEntryColumn::WorkPackage, "WorkPackage"),
        std::make_pair(DLTEntryColumn::LogIndex, "LogIndex"),
        std::make_pair(DLTEntryColumn::SystemTime, "SystemTime"),
        std::make_pair(DLTEntryColumn::MonotonicTime, "MonotonicTime"),
        std::make_pair(DLTEntryColumn::LocalCount, "LocalCount"),
        std::make_pair(DLTEntryColumn::ArgsNo, "ArgsNo"),
        std::make_pair(DLTEntryColumn::EcuID, "EcuID"),
        std::make_pair(DLTEntryColumn::AppID, "AppID"),
        std::make_pair(DLTEntryColumn::CtxID, "CtxID"),
        std::make_pair(DLTEntryColumn::Type, "Type"),
        std::make_pair(DLTEntryColumn::SubType, "SubType"),
        std::make_pair(DLTEntryColumn::Color, "Color"),
        std::make_pair(DLTEntryColumn::Payload, "Payload"),
    };

    enum class DLTFilterColumn {
        Id,         // int: Primary key
        Owner,      // int: User owner primary key
        Name,       // str: Filter name
        Active,     // int: Filter active state (0=inactive, 1=active)
        Type,       // int: Filter type (Positive=0,Negative=1,Marker=2)
        Color,      // int: Log color, see DltEntry
        MsgType,    // int: Log entry type, see DltEntry.Type
        MsgSubType, // int: Log entry type, see DltEntry.SubType
        EcuID,      // str: Log entry ECU ID
        AppID,      // str: Log entry app ID
        CtxID,      // str: Log entry context ID
        Payload     // str: Log entry payload
    };
    const std::map<DLTFilterColumn, std::string> m_dltFilterColumn {
        std::make_pair(DLTFilterColumn::Id, "Id"),
        std::make_pair(DLTFilterColumn::Owner, "Owner"),
        std::make_pair(DLTFilterColumn::Name, "Name"),
        std::make_pair(DLTFilterColumn::Active, "Active"),
        std::make_pair(DLTFilterColumn::Type, "Type"),
        std::make_pair(DLTFilterColumn::Color, "Color"),
        std::make_pair(DLTFilterColumn::MsgType, "MsgType"),
        std::make_pair(DLTFilterColumn::MsgSubType, "MsgSubType"),
        std::make_pair(DLTFilterColumn::EcuID, "EcuID"),
        std::make_pair(DLTFilterColumn::AppID, "AppID"),
        std::make_pair(DLTFilterColumn::CtxID, "CtxID"),
        std::make_pair(DLTFilterColumn::Payload, "Payload"),
    };

    enum class SLGFilterColumn {
        Id,       // int: Primary key
        Owner,    // int: User owner primary key
        Name,     // str: Filter name
        Active,   // int: Filter active state (0=inactive, 1=active)
        Type,     // int: Filter type (Positive=0,Negative=1,Marker=2)
        Color,    // int: Log color, see SLGEntry
        Priority, // int: Log entry type, see SLGEntry.Priority
        Hostname, // str: Log entry hostname
        Payload   // str: Log entry payload
    };
    const std::map<SLGFilterColumn, std::string> m_slgFilterColumn {
        std::make_pair(SLGFilterColumn::Id, "Id"),
        std::make_pair(SLGFilterColumn::Owner, "Owner"),
        std::make_pair(SLGFilterColumn::Name, "Name"),
        std::make_pair(SLGFilterColumn::Type, "Type"),
        std::make_pair(SLGFilterColumn::Active, "Active"),
        std::make_pair(SLGFilterColumn::Color, "Color"),
        std::make_pair(SLGFilterColumn::Priority, "Priority"),
        std::make_pair(SLGFilterColumn::Hostname, "Hostname"),
        std::make_pair(SLGFilterColumn::Payload, "Payload"),
    };

    const std::string m_usersTableName = "tkmUsers";
    const std::string m_accessTokensTableName = "tkmAccessTokens";
    const std::string m_projectsTableName = "tkmProjects";
    const std::string m_workPackagesTableName = "tkmWorkPackages";
    const std::string m_slgEntriesTableName = "tkmSyslogEntries";
    const std::string m_slgFiltersTableName = "tkmSyslogFilters";
    const std::string m_dltEntriesTableName = "tkmDLTEntries";
    const std::string m_dltFiltersTableName = "tkmDLTFilters";
};

static Query tkmQuery {};

} // namespace tkm

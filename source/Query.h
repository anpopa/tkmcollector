/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Query class
 * @details   Provide SQL query strings per database type
 *-
 */

#pragma once

#include <map>
#include <sstream>
#include <string>

#include <taskmonitor/taskmonitor.h>

namespace tkm
{

class Query
{
public:
  enum class Type { SQLite3, PostgreSQL };

  auto createTables(Query::Type type) -> std::string;
  auto dropTables(Query::Type type) -> std::string;

  // Device management
  auto getDevices(Query::Type type) -> std::string;
  auto addDevice(Query::Type type,
                 const std::string &hash,
                 const std::string &name,
                 const std::string &address,
                 int32_t port) -> std::string;
  auto remDevice(Query::Type type, const std::string &hash) -> std::string;
  auto getDevice(Query::Type type, const std::string &hash) -> std::string;
  auto hasDevice(Query::Type type, const std::string &hash) -> std::string;

  // Session management
  auto getSessions(Query::Type type) -> std::string;
  auto getSessions(Query::Type type, const std::string &deviceHash) -> std::string;
  auto addSession(Query::Type type,
                  const tkm::msg::monitor::SessionInfo &sessionInfo,
                  const std::string &deviceHash,
                  uint64_t startTimestamp) -> std::string;
  auto endSession(Query::Type type, const std::string &hash) -> std::string;
  auto remSession(Query::Type type, const std::string &hash) -> std::string;
  auto getSession(Query::Type type, const std::string &hash) -> std::string;
  auto hasSession(Query::Type type, const std::string &hash) -> std::string;

  // Add device data
  auto addData(Query::Type type,
               const std::string &sessionHash,
               const tkm::msg::monitor::SysProcStat &sysProcStat,
               uint64_t systemTime,
               uint64_t monotonicTime,
               uint64_t receiveTime) -> std::string;
  auto addData(Query::Type type,
               const std::string &sessionHash,
               const tkm::msg::monitor::SysProcMemInfo &sysProcMem,
               uint64_t systemTime,
               uint64_t monotonicTime,
               uint64_t receiveTime) -> std::string;
  auto addData(Query::Type type,
               const std::string &sessionHash,
               const tkm::msg::monitor::SysProcDiskStats &sysDiskStats,
               uint64_t systemTime,
               uint64_t monotonicTime,
               uint64_t receiveTime) -> std::string;
  auto addData(Query::Type type,
               const std::string &sessionHash,
               const tkm::msg::monitor::SysProcPressure &sysProcPressure,
               uint64_t systemTime,
               uint64_t monotonicTime,
               uint64_t receiveTime) -> std::string;
  auto addData(Query::Type type,
               const std::string &sessionHash,
               const tkm::msg::monitor::SysProcBuddyInfo &sysProcBuddyInfo,
               uint64_t systemTime,
               uint64_t monotonicTime,
               uint64_t receiveTime) -> std::string;
  auto addData(Query::Type type,
               const std::string &sessionHash,
               const tkm::msg::monitor::SysProcWireless &sysProcWireless,
               uint64_t systemTime,
               uint64_t monotonicTime,
               uint64_t receiveTime) -> std::string;
  auto addData(Query::Type type,
               const std::string &sessionHash,
               const tkm::msg::monitor::ProcAcct &procAcct,
               uint64_t systemTime,
               uint64_t monotonicTime,
               uint64_t receiveTime) -> std::string;
  auto addData(Query::Type type,
               const std::string &sessionHash,
               const tkm::msg::monitor::ProcInfo &procInfo,
               uint64_t systemTime,
               uint64_t monotonicTime,
               uint64_t receiveTime) -> std::string;
  auto addData(Query::Type type,
               const std::string &sessionHash,
               const tkm::msg::monitor::ProcEvent &procEvent,
               uint64_t systemTime,
               uint64_t monotonicTime,
               uint64_t receiveTime) -> std::string;
  auto addData(Query::Type type,
               const std::string &sessionHash,
               const tkm::msg::monitor::ContextInfo &ctxInfo,
               uint64_t systemTime,
               uint64_t monotonicTime,
               uint64_t receiveTime) -> std::string;

public:
  enum class DeviceColumn {
    Id,      // int: Primary key
    Hash,    // str: Unique device hash
    Name,    // str: Device name
    Address, // str: Device address
    Port,    // int: Device port
  };
  const std::map<DeviceColumn, std::string> m_deviceColumn{
      std::make_pair(DeviceColumn::Id, "Id"),
      std::make_pair(DeviceColumn::Hash, "Hash"),
      std::make_pair(DeviceColumn::Name, "Name"),
      std::make_pair(DeviceColumn::Address, "Address"),
      std::make_pair(DeviceColumn::Port, "Port"),
  };

  enum class SessionColumn {
    Id,             // int: Primary key
    Name,           // str: Device name
    Hash,           // str: Unique device hash
    CoreCount,      // int: Number of target CPU cores online
    StartTimestamp, // int: Start timestamp
    EndTimestamp,   // int: End timestamp
    Device,         // int: Device id key
  };
  const std::map<SessionColumn, std::string> m_sessionColumn{
      std::make_pair(SessionColumn::Id, "Id"),
      std::make_pair(SessionColumn::Hash, "Hash"),
      std::make_pair(SessionColumn::Name, "Name"),
      std::make_pair(SessionColumn::CoreCount, "CoreCount"),
      std::make_pair(SessionColumn::StartTimestamp, "StartTimestamp"),
      std::make_pair(SessionColumn::EndTimestamp, "EndTimestamp"),
      std::make_pair(SessionColumn::Device, "Device"),
  };

  enum class ProcEventColumn {
    Id,            // int: Primary key
    SystemTime,    // int: SystemTime
    MonotonicTime, // int: MonotonicTime
    ReceiveTime,   // int: Receive timestamp
    ForkCount,     // int: fork_count
    ExecCount,     // int: exec_count
    ExitCount,     // int: exit_count
    UIdCount,      // int: uid_count
    GIdCount,      // int: gid_count
    SessionId,     // int: Session id key
  };
  const std::map<ProcEventColumn, std::string> m_procEventColumn{
      std::make_pair(ProcEventColumn::Id, "Id"),
      std::make_pair(ProcEventColumn::SystemTime, "SystemTime"),
      std::make_pair(ProcEventColumn::MonotonicTime, "MonotonicTime"),
      std::make_pair(ProcEventColumn::ReceiveTime, "ReceiveTime"),
      std::make_pair(ProcEventColumn::ForkCount, "ForkCount"),
      std::make_pair(ProcEventColumn::ExecCount, "ExecCount"),
      std::make_pair(ProcEventColumn::ExitCount, "ExitCount"),
      std::make_pair(ProcEventColumn::UIdCount, "UIdCount"),
      std::make_pair(ProcEventColumn::GIdCount, "GIdCount"),
      std::make_pair(ProcEventColumn::SessionId, "SessionId"),
  };

  enum class SysProcStatColumn {
    Id,            // int: Primary key
    SystemTime,    // int: SystemTime
    MonotonicTime, // int: MonotonicTime
    ReceiveTime,   // int: Receive timestamp
    CPUStatName,   // str: CPUStat.name
    CPUStatAll,    // int: CPUStat.all
    CPUStatUsr,    // int: CPUStat.usr
    CPUStatSys,    // int: CPUStat.sys
    CPUStatIow,    // int: CPUStat.iow
    SessionId,     // int: Session id key
  };
  const std::map<SysProcStatColumn, std::string> m_sysProcStatColumn{
      std::make_pair(SysProcStatColumn::Id, "Id"),
      std::make_pair(SysProcStatColumn::SystemTime, "SystemTime"),
      std::make_pair(SysProcStatColumn::MonotonicTime, "MonotonicTime"),
      std::make_pair(SysProcStatColumn::ReceiveTime, "ReceiveTime"),
      std::make_pair(SysProcStatColumn::CPUStatName, "CPUStatName"),
      std::make_pair(SysProcStatColumn::CPUStatAll, "CPUStatAll"),
      std::make_pair(SysProcStatColumn::CPUStatUsr, "CPUStatUsr"),
      std::make_pair(SysProcStatColumn::CPUStatSys, "CPUStatSys"),
      std::make_pair(SysProcStatColumn::CPUStatIow, "CPUStatIow"),
      std::make_pair(SysProcStatColumn::SessionId, "SessionId"),
  };

  enum class SysProcMemColumn {
    Id,              // int: Primary key
    SystemTime,      // int: SystemTime
    MonotonicTime,   // int: MonotonicTime
    ReceiveTime,     // int: Receive timestamp
    MemTotal,        // int: MemTotal
    MemFree,         // int: MemFree
    MemAvail,        // int: MemAvail
    MemCached,       // int: MemCached
    MemAvailPercent, // int: MemAvailPercent
    SwapTotal,       // int: SwapTotal
    SwapFree,        // int: SwapFree
    SwapCached,      // int: SwapCached
    SwapFreePercent, // int: SwapFreePercent
    CmaTotal,        // int: CmaTotal
    CmaFree,         // int: CmaFree
    SessionId,       // int: Session id key
  };
  const std::map<SysProcMemColumn, std::string> m_sysProcMemColumn{
      std::make_pair(SysProcMemColumn::Id, "Id"),
      std::make_pair(SysProcMemColumn::SystemTime, "SystemTime"),
      std::make_pair(SysProcMemColumn::MonotonicTime, "MonotonicTime"),
      std::make_pair(SysProcMemColumn::ReceiveTime, "ReceiveTime"),
      std::make_pair(SysProcMemColumn::MemTotal, "MemTotal"),
      std::make_pair(SysProcMemColumn::MemFree, "MemFree"),
      std::make_pair(SysProcMemColumn::MemAvail, "MemAvail"),
      std::make_pair(SysProcMemColumn::MemCached, "MemCached"),
      std::make_pair(SysProcMemColumn::MemAvailPercent, "MemAvailPercent"),
      std::make_pair(SysProcMemColumn::SwapTotal, "SwapTotal"),
      std::make_pair(SysProcMemColumn::SwapFree, "SwapFree"),
      std::make_pair(SysProcMemColumn::SwapCached, "SwapCached"),
      std::make_pair(SysProcMemColumn::SwapFreePercent, "SwapFreePercent"),
      std::make_pair(SysProcMemColumn::CmaTotal, "CmaTotal"),
      std::make_pair(SysProcMemColumn::CmaFree, "CmaFree"),
      std::make_pair(SysProcMemColumn::SessionId, "SessionId"),
  };

  enum class SysProcDiskColumn {
    Id,              // int: Primary key
    SystemTime,      // int: SystemTime
    MonotonicTime,   // int: MonotonicTime
    ReceiveTime,     // int: Receive timestamp
    Major,           // int: Device major number
    Minor,           // int: Device minor number
    Name,            // str: Device name
    ReadsCompleted,  // int: reads completed
    ReadsMerged,     // int: reads merged
    ReadsSpentMs,    // int: reads spent ms
    WritesCompleted, // int: writes completed
    WritesMerged,    // int: writes merged
    WritesSpentMs,   // int: writes spent ms
    IOInProgress,    // int: io in progress
    IOSpentMs,       // int: io spent ms
    IOWeightedMs,    // int: io weighted ms
    SessionId,       // int: Session id key
  };
  const std::map<SysProcDiskColumn, std::string> m_sysProcDiskColumn{
      std::make_pair(SysProcDiskColumn::Id, "Id"),
      std::make_pair(SysProcDiskColumn::SystemTime, "SystemTime"),
      std::make_pair(SysProcDiskColumn::MonotonicTime, "MonotonicTime"),
      std::make_pair(SysProcDiskColumn::ReceiveTime, "ReceiveTime"),
      std::make_pair(SysProcDiskColumn::Major, "Major"),
      std::make_pair(SysProcDiskColumn::Minor, "Minor"),
      std::make_pair(SysProcDiskColumn::Name, "Name"),
      std::make_pair(SysProcDiskColumn::ReadsCompleted, "ReadsCompleted"),
      std::make_pair(SysProcDiskColumn::ReadsMerged, "ReadsMerged"),
      std::make_pair(SysProcDiskColumn::ReadsSpentMs, "ReadsSpent"),
      std::make_pair(SysProcDiskColumn::WritesCompleted, "WritesCompleted"),
      std::make_pair(SysProcDiskColumn::WritesMerged, "WritesMerged"),
      std::make_pair(SysProcDiskColumn::WritesSpentMs, "WritesSpent"),
      std::make_pair(SysProcDiskColumn::IOInProgress, "IOInProgress"),
      std::make_pair(SysProcDiskColumn::IOSpentMs, "IOSpent"),
      std::make_pair(SysProcDiskColumn::IOWeightedMs, "IOWeightedMs"),
      std::make_pair(SysProcDiskColumn::SessionId, "SessionId"),
  };

  enum class SysProcPressureColumn {
    Id,            // int: Primary key
    SystemTime,    // int: SystemTime
    MonotonicTime, // int: MonotonicTime
    ReceiveTime,   // int: Receive timestamp
    CPUSomeAvg10,  // float: cpu_some_avg10
    CPUSomeAvg60,  // float: cpu_some_avg60
    CPUSomeAvg300, // float: cpu_some_avg300
    CPUSomeTotal,  // float: cpu_some_total
    CPUFullAvg10,  // float: cpu_full_avg10
    CPUFullAvg60,  // float: cpu_full_avg60
    CPUFullAvg300, // float: cpu_full_avg300
    CPUFullTotal,  // float: cpu_full_total
    MEMSomeAvg10,  // float: mem_some_avg10
    MEMSomeAvg60,  // float: mem_some_avg60
    MEMSomeAvg300, // float: mem_some_avg300
    MEMSomeTotal,  // float: mem_some_total
    MEMFullAvg10,  // float: mem_full_avg10
    MEMFullAvg60,  // float: mem_full_avg60
    MEMFullAvg300, // float: mem_full_avg300
    MEMFullTotal,  // float: mem_full_total
    IOSomeAvg10,   // float: io_some_avg10
    IOSomeAvg60,   // float: io_some_avg60
    IOSomeAvg300,  // float: io_some_avg300
    IOSomeTotal,   // float: io_some_total
    IOFullAvg10,   // float: io_full_avg10
    IOFullAvg60,   // float: io_full_avg60
    IOFullAvg300,  // float: io_full_avg300
    IOFullTotal,   // float: io_full_total
    SessionId,     // int: Session id key
  };
  const std::map<SysProcPressureColumn, std::string> m_sysProcPressureColumn{
      std::make_pair(SysProcPressureColumn::Id, "Id"),
      std::make_pair(SysProcPressureColumn::SystemTime, "SystemTime"),
      std::make_pair(SysProcPressureColumn::MonotonicTime, "MonotonicTime"),
      std::make_pair(SysProcPressureColumn::ReceiveTime, "ReceiveTime"),
      std::make_pair(SysProcPressureColumn::CPUSomeAvg10, "CPUSomeAvg10"),
      std::make_pair(SysProcPressureColumn::CPUSomeAvg60, "CPUSomeAvg60"),
      std::make_pair(SysProcPressureColumn::CPUSomeAvg300, "CPUSomeAvg300"),
      std::make_pair(SysProcPressureColumn::CPUSomeTotal, "CPUSomeTotal"),
      std::make_pair(SysProcPressureColumn::CPUFullAvg10, "CPUFullAvg10"),
      std::make_pair(SysProcPressureColumn::CPUFullAvg60, "CPUFullAvg60"),
      std::make_pair(SysProcPressureColumn::CPUFullAvg300, "CPUFullAvg300"),
      std::make_pair(SysProcPressureColumn::CPUFullTotal, "CPUFullTotal"),
      std::make_pair(SysProcPressureColumn::MEMSomeAvg10, "MEMSomeAvg10"),
      std::make_pair(SysProcPressureColumn::MEMSomeAvg60, "MEMSomeAvg60"),
      std::make_pair(SysProcPressureColumn::MEMSomeAvg300, "MEMSomeAvg300"),
      std::make_pair(SysProcPressureColumn::MEMSomeTotal, "MEMSomeTotal"),
      std::make_pair(SysProcPressureColumn::MEMFullAvg10, "MEMFullAvg10"),
      std::make_pair(SysProcPressureColumn::MEMFullAvg60, "MEMFullAvg60"),
      std::make_pair(SysProcPressureColumn::MEMFullAvg300, "MEMFullAvg300"),
      std::make_pair(SysProcPressureColumn::MEMFullTotal, "MEMFullTotal"),
      std::make_pair(SysProcPressureColumn::IOSomeAvg10, "IOSomeAvg10"),
      std::make_pair(SysProcPressureColumn::IOSomeAvg60, "IOSomeAvg60"),
      std::make_pair(SysProcPressureColumn::IOSomeAvg300, "IOSomeAvg300"),
      std::make_pair(SysProcPressureColumn::IOSomeTotal, "IOSomeTotal"),
      std::make_pair(SysProcPressureColumn::IOFullAvg10, "IOFullAvg10"),
      std::make_pair(SysProcPressureColumn::IOFullAvg60, "IOFullAvg60"),
      std::make_pair(SysProcPressureColumn::IOFullAvg300, "IOFullAvg300"),
      std::make_pair(SysProcPressureColumn::IOFullTotal, "IOFullTotal"),
      std::make_pair(SysProcPressureColumn::SessionId, "SessionId"),
  };

  enum class ProcAcctColumn {
    Id,                    // int: Primary key
    SystemTime,            // int: SystemTime
    MonotonicTime,         // int: MonotonicTime
    ReceiveTime,           // int: Receive timestamp
    AcComm,                // str: ac_comm
    AcUid,                 // int: ac_uid
    AcGid,                 // int: ac_gid
    AcPid,                 // int: ac_pid
    AcPPid,                // int: ac_ppid
    AcUTime,               // int: ac_utime
    AcSTime,               // int: ac_stime
    CpuCount,              // int: cpu_count
    CpuRunRealTotal,       // int: cpu_run_real_total
    CpuRunVirtualTotal,    // int: cpu_run_virtual_total
    CpuDelayTotal,         // int: cpu_delay_total
    CpuDelayAverage,       // int: cpu_delay_average
    CoreMem,               // int: coremem
    VirtMem,               // int: virtmem
    HiwaterRss,            // int: hiwater_rss
    HiwaterVm,             // int: hiwater_vm
    Nvcsw,                 // int: nvcsw
    Nivcsw,                // int: nivcsw
    SwapinCount,           // int: swapin_count
    SwapinDelayTotal,      // int: swapin_delay_total
    SwapinDelayAverage,    // int: swapin_delay_average
    BlkIOCount,            // int: blkio_count
    BlkIODelayTotal,       // int: blkio_delay_total
    BlkIODelayAverage,     // int: blkio_delay_average
    IOStorageReadBytes,    // int: Storage read bytes
    IOStorageWriteBytes,   // int: Storage write bytes
    IOReadChar,            // int: IO read bytes
    IOWriteChar,           // int: IO write bytes
    IOReadSyscalls,        // int: IO read syscalls
    IOWriteSyscalls,       // int: IO write syscalls
    FreePagesCount,        // int: freepages_count
    FreePagesDelayTotal,   // int: freepages_delay_total
    FreePagesDelayAverage, // int: freepages_delay_average
    ThrashingCount,        // int: thrashing_count
    ThrashingDelayTotal,   // int: thrashing_delay_total
    ThrashingDelayAverage, // int: thrashing_delay_average
    SessionId,             // int: Session id key
  };
  const std::map<ProcAcctColumn, std::string> m_procAcctColumn{
      std::make_pair(ProcAcctColumn::Id, "Id"),
      std::make_pair(ProcAcctColumn::SystemTime, "SystemTime"),
      std::make_pair(ProcAcctColumn::MonotonicTime, "MonotonicTime"),
      std::make_pair(ProcAcctColumn::ReceiveTime, "ReceiveTime"),
      std::make_pair(ProcAcctColumn::AcComm, "AcComm"),
      std::make_pair(ProcAcctColumn::AcUid, "AcUid"),
      std::make_pair(ProcAcctColumn::AcGid, "AcGid"),
      std::make_pair(ProcAcctColumn::AcPid, "AcPid"),
      std::make_pair(ProcAcctColumn::AcPPid, "AcPPid"),
      std::make_pair(ProcAcctColumn::AcUTime, "AcUTime"),
      std::make_pair(ProcAcctColumn::AcSTime, "AcSTime"),
      std::make_pair(ProcAcctColumn::CpuCount, "CpuCount"),
      std::make_pair(ProcAcctColumn::CpuRunRealTotal, "CpuRunRealTotal"),
      std::make_pair(ProcAcctColumn::CpuRunVirtualTotal, "CpuRunVirtualTotal"),
      std::make_pair(ProcAcctColumn::CpuDelayTotal, "CpuDelayTotal"),
      std::make_pair(ProcAcctColumn::CpuDelayAverage, "CpuDelayAverage"),
      std::make_pair(ProcAcctColumn::CoreMem, "CoreMem"),
      std::make_pair(ProcAcctColumn::VirtMem, "VirtMem"),
      std::make_pair(ProcAcctColumn::HiwaterRss, "HiwaterRss"),
      std::make_pair(ProcAcctColumn::HiwaterVm, "HiwaterVm"),
      std::make_pair(ProcAcctColumn::Nvcsw, "Nvcsw"),
      std::make_pair(ProcAcctColumn::Nivcsw, "Nivcsw"),
      std::make_pair(ProcAcctColumn::SwapinCount, "SwapinCount"),
      std::make_pair(ProcAcctColumn::SwapinDelayTotal, "SwapinDelayTotal"),
      std::make_pair(ProcAcctColumn::SwapinDelayAverage, "SwapinDelayAverage"),
      std::make_pair(ProcAcctColumn::BlkIOCount, "BlkIOCount"),
      std::make_pair(ProcAcctColumn::BlkIODelayTotal, "BlkIODelayTotal"),
      std::make_pair(ProcAcctColumn::BlkIODelayAverage, "BlkIODelayAverage"),
      std::make_pair(ProcAcctColumn::IOStorageReadBytes, "IOStorageReadBytes"),
      std::make_pair(ProcAcctColumn::IOStorageWriteBytes, "IOStorageWriteBytes"),
      std::make_pair(ProcAcctColumn::IOReadChar, "IOReadChar"),
      std::make_pair(ProcAcctColumn::IOWriteChar, "IOWriteChar"),
      std::make_pair(ProcAcctColumn::IOReadSyscalls, "IOReadSyscalls"),
      std::make_pair(ProcAcctColumn::IOWriteSyscalls, "IOWriteSyscalls"),
      std::make_pair(ProcAcctColumn::FreePagesCount, "FreePagesCount"),
      std::make_pair(ProcAcctColumn::FreePagesDelayTotal, "FreePagesDelayTotal"),
      std::make_pair(ProcAcctColumn::FreePagesDelayAverage, "FreePagesDelayAverage"),
      std::make_pair(ProcAcctColumn::ThrashingCount, "ThrashingCount"),
      std::make_pair(ProcAcctColumn::ThrashingDelayTotal, "ThrashingDelayTotal"),
      std::make_pair(ProcAcctColumn::ThrashingDelayAverage, "ThrashingDelayAverage"),
      std::make_pair(ProcAcctColumn::SessionId, "SessionId"),
  };

  enum class ProcInfoColumn {
    Id,            // int: Primary key
    SystemTime,    // int: SystemTime
    MonotonicTime, // int: MonotonicTime
    ReceiveTime,   // int: Receive timestamp
    Comm,          // str: Process name
    Pid,           // int: Process pid
    PPid,          // int: Process ppid
    CtxId,         // int: Context id
    CtxName,       // str: Context name
    CpuTime,       // int: Total cpu time
    CpuPercent,    // int: Cpu percent in interval
    MemVmSize,     // int: VmSize memory usage
    MemVmRSS,      // int: VmRSS memory usage
    MemShared,     // int: Shared memory usage
    SessionId,     // int: Session id key
  };
  const std::map<ProcInfoColumn, std::string> m_procInfoColumn{
      std::make_pair(ProcInfoColumn::Id, "Id"),
      std::make_pair(ProcInfoColumn::SystemTime, "SystemTime"),
      std::make_pair(ProcInfoColumn::MonotonicTime, "MonotonicTime"),
      std::make_pair(ProcInfoColumn::ReceiveTime, "ReceiveTime"),
      std::make_pair(ProcInfoColumn::Comm, "Comm"),
      std::make_pair(ProcInfoColumn::Pid, "PID"),
      std::make_pair(ProcInfoColumn::PPid, "PPID"),
      std::make_pair(ProcInfoColumn::CtxId, "ContextId"),
      std::make_pair(ProcInfoColumn::CtxName, "ContextName"),
      std::make_pair(ProcInfoColumn::CpuTime, "CpuTime"),
      std::make_pair(ProcInfoColumn::CpuPercent, "CpuPercent"),
      std::make_pair(ProcInfoColumn::MemVmSize, "VmSize"),
      std::make_pair(ProcInfoColumn::MemVmRSS, "VmRSS"),
      std::make_pair(ProcInfoColumn::MemShared, "MemShared"),
      std::make_pair(ProcInfoColumn::SessionId, "SessionId"),
  };

  enum class ContextInfoColumn {
    Id,              // int: Primary key
    SystemTime,      // int: SystemTime
    MonotonicTime,   // int: MonotonicTime
    ReceiveTime,     // int: Receive timestamp
    CtxId,           // int: Context id
    CtxName,         // str: Context name
    TotalCpuTime,    // int: Total cpu time
    TotalCpuPercent, // int: Total Cpu percent in interval
    TotalMemVmRSS,   // int: Total VmRSS memory usage
    TotalMemShared,  // int: Total processes shared memory
    SessionId,       // int: Session id key
  };
  const std::map<ContextInfoColumn, std::string> m_contextInfoColumn{
      std::make_pair(ContextInfoColumn::Id, "Id"),
      std::make_pair(ContextInfoColumn::SystemTime, "SystemTime"),
      std::make_pair(ContextInfoColumn::MonotonicTime, "MonotonicTime"),
      std::make_pair(ContextInfoColumn::ReceiveTime, "ReceiveTime"),
      std::make_pair(ContextInfoColumn::CtxId, "ContextId"),
      std::make_pair(ContextInfoColumn::CtxName, "ContextName"),
      std::make_pair(ContextInfoColumn::TotalCpuTime, "TotalCpuTime"),
      std::make_pair(ContextInfoColumn::TotalCpuPercent, "TotalCpuPercent"),
      std::make_pair(ContextInfoColumn::TotalMemVmRSS, "TotalVmRSS"),
      std::make_pair(ContextInfoColumn::TotalMemShared, "TotalMemShared"),
      std::make_pair(ContextInfoColumn::SessionId, "SessionId"),
  };

  enum class SysProcBuddyInfoColumn {
    Id,            // int: Primary key
    SystemTime,    // int: SystemTime
    MonotonicTime, // int: MonotonicTime
    ReceiveTime,   // int: Receive timestamp
    Name,          // str: BuddyInfo.name
    Zone,          // str: BuddyInfo.zone
    Data,          // str: BuddyInfo.data
    SessionId,     // int: Session id key
  };
  const std::map<SysProcBuddyInfoColumn, std::string> m_sysProcBuddyInfoColumn{
      std::make_pair(SysProcBuddyInfoColumn::Id, "Id"),
      std::make_pair(SysProcBuddyInfoColumn::SystemTime, "SystemTime"),
      std::make_pair(SysProcBuddyInfoColumn::MonotonicTime, "MonotonicTime"),
      std::make_pair(SysProcBuddyInfoColumn::ReceiveTime, "ReceiveTime"),
      std::make_pair(SysProcBuddyInfoColumn::Name, "Name"),
      std::make_pair(SysProcBuddyInfoColumn::Zone, "Zone"),
      std::make_pair(SysProcBuddyInfoColumn::Data, "Data"),
      std::make_pair(SysProcBuddyInfoColumn::SessionId, "SessionId"),
  };

  enum class SysProcWirelessColumn {
    Id,             // int: Primary key
    SystemTime,     // int: SystemTime
    MonotonicTime,  // int: MonotonicTime
    ReceiveTime,    // int: Receive timestamp
    Name,           // str: WlanInterface.name
    Status,         // str: WlanInterface.status
    QualityLink,    // int: WlanInterface.quality_link
    QualityLevel,   // int: WlanInterface.quality_level
    QualityNoise,   // int: WlanInterface.quality_noise
    DiscardedNWId,  // int: WlanInterface.discarded_nwid
    DiscardedCrypt, // int: WlanInterface.discarded_crypt
    DiscardedFrag,  // int: WlanInterface.discarded_frag
    DiscardedRetry, // int: WlanInterface.discarded_retry
    DiscardedMisc,  // int: WlanInterface.discarded_misc
    MissedBeacon,   // int: WlanInterface.missed_beacon
    SessionId,      // int: Session id key
  };
  const std::map<SysProcWirelessColumn, std::string> m_sysProcWirelessColumn{
      std::make_pair(SysProcWirelessColumn::Id, "Id"),
      std::make_pair(SysProcWirelessColumn::SystemTime, "SystemTime"),
      std::make_pair(SysProcWirelessColumn::MonotonicTime, "MonotonicTime"),
      std::make_pair(SysProcWirelessColumn::ReceiveTime, "ReceiveTime"),
      std::make_pair(SysProcWirelessColumn::Name, "Name"),
      std::make_pair(SysProcWirelessColumn::Status, "Status"),
      std::make_pair(SysProcWirelessColumn::QualityLink, "QualityLink"),
      std::make_pair(SysProcWirelessColumn::QualityLevel, "QualityLevel"),
      std::make_pair(SysProcWirelessColumn::QualityNoise, "QualityNoise"),
      std::make_pair(SysProcWirelessColumn::DiscardedNWId, "DiscardedNWId"),
      std::make_pair(SysProcWirelessColumn::DiscardedCrypt, "DiscardedCrypt"),
      std::make_pair(SysProcWirelessColumn::DiscardedFrag, "DiscardedFrag"),
      std::make_pair(SysProcWirelessColumn::DiscardedRetry, "DiscardedRetry"),
      std::make_pair(SysProcWirelessColumn::DiscardedMisc, "DiscardedMisc"),
      std::make_pair(SysProcWirelessColumn::MissedBeacon, "MissedBeacon"),
      std::make_pair(SysProcWirelessColumn::SessionId, "SessionId"),
  };

  const std::string m_devicesTableName = "tkmDevices";
  const std::string m_sessionsTableName = "tkmSessions";
  const std::string m_sysProcStatTableName = "tkmSysProcStat";
  const std::string m_sysProcMemInfoTableName = "tkmSysProcMemInfo";
  const std::string m_sysProcDiskStatsTableName = "tkmSysProcDiskStats";
  const std::string m_sysProcPressureTableName = "tkmSysProcPressure";
  const std::string m_sysProcBuddyInfoTableName = "tkmSysProcBuddyInfo";
  const std::string m_sysProcWirelessTableName = "tkmSysProcWireless";
  const std::string m_procAcctTableName = "tkmProcAcct";
  const std::string m_procInfoTableName = "tkmProcInfo";
  const std::string m_procEventTableName = "tkmProcEvent";
  const std::string m_contextInfoTableName = "tkmContextInfo";
};

static Query tkmQuery{};

} // namespace tkm

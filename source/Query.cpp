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

#include "Query.h"

namespace tkm
{

auto Query::createTables(Query::Type type) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    // Devices table
    out << "CREATE TABLE IF NOT EXISTS " << m_devicesTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_deviceColumn.at(DeviceColumn::Id) << " INTEGER PRIMARY KEY, ";
    } else {
      out << m_deviceColumn.at(DeviceColumn::Id) << " SERIAL PRIMARY KEY, ";
    }
    out << m_deviceColumn.at(DeviceColumn::Hash) << " TEXT NOT NULL, "
        << m_deviceColumn.at(DeviceColumn::Name) << " TEXT NOT NULL, "
        << m_deviceColumn.at(DeviceColumn::Address) << " TEXT NOT NULL, "
        << m_deviceColumn.at(DeviceColumn::Port) << " INTEGER NOT NULL);";

    // Sessions table
    out << "CREATE TABLE IF NOT EXISTS " << m_sessionsTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_sessionColumn.at(SessionColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_sessionColumn.at(SessionColumn::Name) << " TEXT NOT NULL, "
          << m_sessionColumn.at(SessionColumn::Hash) << " TEXT NOT NULL, "
          << m_sessionColumn.at(SessionColumn::CoreCount) << " INTEGER NOT NULL, "
          << m_sessionColumn.at(SessionColumn::StartTimestamp) << " INTEGER NOT NULL, "
          << m_sessionColumn.at(SessionColumn::EndTimestamp) << " INTEGER NOT NULL, "
          << m_sessionColumn.at(SessionColumn::Device) << " INTEGER NOT NULL, ";
    } else {
      out << m_sessionColumn.at(SessionColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_sessionColumn.at(SessionColumn::Name) << " TEXT NOT NULL, "
          << m_sessionColumn.at(SessionColumn::Hash) << " TEXT NOT NULL, "
          << m_sessionColumn.at(SessionColumn::CoreCount) << " BIGINT NOT NULL, "
          << m_sessionColumn.at(SessionColumn::StartTimestamp) << " BIGINT NOT NULL, "
          << m_sessionColumn.at(SessionColumn::EndTimestamp) << " BIGINT NOT NULL, "
          << m_sessionColumn.at(SessionColumn::Device) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFDevice FOREIGN KEY(" << m_sessionColumn.at(SessionColumn::Device)
        << ") REFERENCES " << m_devicesTableName << "(" << m_deviceColumn.at(DeviceColumn::Id)
        << ") ON DELETE CASCADE);";

    // ProcEvent table
    out << "CREATE TABLE IF NOT EXISTS " << m_procEventTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_procEventColumn.at(ProcEventColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_procEventColumn.at(ProcEventColumn::SystemTime) << " INTEGER NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::MonotonicTime) << " INTEGER NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::ReceiveTime) << " INTEGER NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::ForkCount) << " INTEGER NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::ExecCount) << " INTEGER NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::ExitCount) << " INTEGER NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::UIdCount) << " INTEGER NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::GIdCount) << " INTEGER NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::SessionId) << " INTEGER NOT NULL, ";
    } else {
      out << m_procEventColumn.at(ProcEventColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_procEventColumn.at(ProcEventColumn::SystemTime) << " BIGINT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::MonotonicTime) << " BIGINT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::ReceiveTime) << " BIGINT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::ForkCount) << " BIGINT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::ExecCount) << " BIGINT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::ExitCount) << " BIGINT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::UIdCount) << " BIGINT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::GIdCount) << " BIGINT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::SessionId) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY(" << m_procEventColumn.at(ProcEventColumn::SessionId)
        << ") REFERENCES " << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // SysProcStat table
    out << "CREATE TABLE IF NOT EXISTS " << m_sysProcStatTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_sysProcStatColumn.at(SysProcStatColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_sysProcStatColumn.at(SysProcStatColumn::SystemTime) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::MonotonicTime) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::ReceiveTime) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatName) << " TEXT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatAll) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatUsr) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatSys) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatIow) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << " INTEGER NOT NULL, ";
    } else {
      out << m_sysProcStatColumn.at(SysProcStatColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_sysProcStatColumn.at(SysProcStatColumn::SystemTime) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::MonotonicTime) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::ReceiveTime) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatName) << " TEXT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatAll) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatUsr) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatSys) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatIow) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY("
        << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << ") REFERENCES "
        << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // SysProcMemInfo table
    out << "CREATE TABLE IF NOT EXISTS " << m_sysProcMemInfoTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_sysProcMemColumn.at(SysProcMemColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SystemTime) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MonotonicTime) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::ReceiveTime) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemTotal) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemFree) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemAvail) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemCached) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemAvailPercent) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::Active) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::Inactive) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::Slab) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::KReclaimable) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SReclaimable) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SUnreclaim) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::KernelStack) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapTotal) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapFree) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapCached) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapFreePercent) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::CmaTotal) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::CmaFree) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SessionId) << " INTEGER NOT NULL, ";
    } else {
      out << m_sysProcMemColumn.at(SysProcMemColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SystemTime) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MonotonicTime) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::ReceiveTime) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemTotal) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemFree) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemAvail) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemCached) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemAvailPercent) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::Active) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::Inactive) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::Slab) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::KReclaimable) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SReclaimable) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SUnreclaim) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::KernelStack) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapTotal) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapFree) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapCached) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapFreePercent) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::CmaTotal) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::CmaFree) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SessionId) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY(" << m_sysProcMemColumn.at(SysProcMemColumn::SessionId)
        << ") REFERENCES " << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // SysProcDiskStats table
    out << "CREATE TABLE IF NOT EXISTS " << m_sysProcDiskStatsTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_sysProcDiskColumn.at(SysProcDiskColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::SystemTime) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::MonotonicTime) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReceiveTime) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::Major) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::Minor) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::Name) << " TEXT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReadsCompleted) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReadsMerged) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReadsSpentMs) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::WritesCompleted) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::WritesMerged) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::WritesSpentMs) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::IOInProgress) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::IOSpentMs) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::IOWeightedMs) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::SessionId) << " INTEGER NOT NULL, ";
    } else {
      out << m_sysProcDiskColumn.at(SysProcDiskColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::SystemTime) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::MonotonicTime) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReceiveTime) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::Major) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::Minor) << " INTEGER NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::Name) << " TEXT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReadsCompleted) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReadsMerged) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReadsSpentMs) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::WritesCompleted) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::WritesMerged) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::WritesSpentMs) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::IOInProgress) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::IOSpentMs) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::IOWeightedMs) << " BIGINT NOT NULL, "
          << m_sysProcDiskColumn.at(SysProcDiskColumn::SessionId) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY("
        << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << ") REFERENCES "
        << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // SysProcPressure table
    out << "CREATE TABLE IF NOT EXISTS " << m_sysProcPressureTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_sysProcPressureColumn.at(SysProcPressureColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::SystemTime) << " INTEGER NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MonotonicTime)
          << " INTEGER NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::ReceiveTime)
          << " INTEGER NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg10) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg60) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg300) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeTotal)
          << " INTEGER NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg10) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg60) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg300) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullTotal)
          << " INTEGER NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg10) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg60) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg300) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeTotal)
          << " INTEGER NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg10) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg60) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg300) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullTotal)
          << " INTEGER NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg10)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg60)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg300)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeTotal)
          << " INTEGER NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg10)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg60)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg300)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullTotal)
          << " INTEGER NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::SessionId)
          << " INTEGER NOT NULL, ";
    } else {
      out << m_sysProcPressureColumn.at(SysProcPressureColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::SystemTime) << " BIGINT NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MonotonicTime)
          << " BIGINT NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::ReceiveTime)
          << " BIGINT NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg10)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg60)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg300)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeTotal)
          << " BIGINT NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg10)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg60)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg300)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullTotal)
          << " BIGINT NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg10)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg60)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg300)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeTotal)
          << " BIGINT NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg10)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg60)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg300)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullTotal)
          << " BIGINT NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg10)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg60)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg300)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeTotal)
          << " BIGINT NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg10)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg60)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg300)
          << " REAL NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullTotal)
          << " BIGINT NOT NULL, " << m_sysProcPressureColumn.at(SysProcPressureColumn::SessionId)
          << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY("
        << m_sysProcPressureColumn.at(SysProcPressureColumn::SessionId) << ") REFERENCES "
        << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // SysProcVMStat table
    out << "CREATE TABLE IF NOT EXISTS " << m_sysProcVMStatTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_sysProcVMStatColumn.at(SysProcVMStatColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::SystemTime) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::MonotonicTime) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ReceiveTime) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGpgin) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGpgout) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PSwpin) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PSwpout) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGmajfault) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGreuse) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealKswapd) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealDirect) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealKhugepaged) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealAnon) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealFile) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanKswapd) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanDirect) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanKhugepaged) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanDirectThrottle) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanAnon) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanFile) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::OOMKill) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::CompactStall) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::CompactFail) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::CompactSuccess) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpFaultAlloc) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpCollapseAlloc) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpCollapseAllocFailed) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpFileAlloc) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpFileMapped) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSplitPage) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSplitPageFailed) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpZeroPageAlloc) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpZeroPageAllocFailed) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSwpout) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSwpoutFallback) << " INTEGER NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::SessionId) << " INTEGER NOT NULL, ";
    } else {
      out << m_sysProcVMStatColumn.at(SysProcVMStatColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::SystemTime) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::MonotonicTime) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ReceiveTime) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGpgin) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGpgout) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PSwpin) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PSwpout) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGmajfault) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGreuse) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealKswapd) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealDirect) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealKhugepaged) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealAnon) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealFile) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanKswapd) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanDirect) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanKhugepaged) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanDirectThrottle) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanAnon) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanFile) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::OOMKill) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::CompactStall) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::CompactFail) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::CompactSuccess) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpFaultAlloc) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpCollapseAlloc) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpCollapseAllocFailed) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpFileAlloc) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpFileMapped) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSplitPage) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSplitPageFailed) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpZeroPageAlloc) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpZeroPageAllocFailed) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSwpout) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSwpoutFallback) << " BIGINT NOT NULL, "
          << m_sysProcVMStatColumn.at(SysProcVMStatColumn::SessionId) << " BIGINT NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY(" << m_sysProcVMStatColumn.at(SysProcVMStatColumn::SessionId)
        << ") REFERENCES " << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // ProcAcct table
    out << "CREATE TABLE IF NOT EXISTS " << m_procAcctTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_procAcctColumn.at(ProcAcctColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_procAcctColumn.at(ProcAcctColumn::SystemTime) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::MonotonicTime) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::ReceiveTime) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcComm) << " TEXT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcUid) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcGid) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcPid) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcPPid) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcUTime) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcSTime) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuCount) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuRunRealTotal) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuRunVirtualTotal) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuDelayTotal) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuDelayAverage) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CoreMem) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::VirtMem) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::HiwaterRss) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::HiwaterVm) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::Nvcsw) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::Nivcsw) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::SwapinCount) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::SwapinDelayTotal) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::SwapinDelayAverage) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::BlkIOCount) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::BlkIODelayTotal) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::BlkIODelayAverage) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOStorageReadBytes) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOStorageWriteBytes) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOReadChar) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOWriteChar) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOReadSyscalls) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOWriteSyscalls) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::FreePagesCount) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::FreePagesDelayTotal) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::FreePagesDelayAverage) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::ThrashingCount) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::ThrashingDelayTotal) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::ThrashingDelayAverage) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::SessionId) << " INTEGER NOT NULL, ";
    } else {
      out << m_procAcctColumn.at(ProcAcctColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_procAcctColumn.at(ProcAcctColumn::SystemTime) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::MonotonicTime) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::ReceiveTime) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcComm) << " TEXT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcUid) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcGid) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcPid) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcPPid) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcUTime) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcSTime) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuCount) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuRunRealTotal) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuRunVirtualTotal) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuDelayTotal) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuDelayAverage) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CoreMem) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::VirtMem) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::HiwaterRss) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::HiwaterVm) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::Nvcsw) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::Nivcsw) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::SwapinCount) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::SwapinDelayTotal) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::SwapinDelayAverage) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::BlkIOCount) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::BlkIODelayTotal) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::BlkIODelayAverage) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOStorageReadBytes) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOStorageWriteBytes) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOReadChar) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOWriteChar) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOReadSyscalls) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::IOWriteSyscalls) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::FreePagesCount) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::FreePagesDelayTotal) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::FreePagesDelayAverage) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::ThrashingCount) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::ThrashingDelayTotal) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::ThrashingDelayAverage) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::SessionId) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY(" << m_procAcctColumn.at(ProcAcctColumn::SessionId)
        << ") REFERENCES " << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // ProcInfo table
    out << "CREATE TABLE IF NOT EXISTS " << m_procInfoTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_procInfoColumn.at(ProcInfoColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_procInfoColumn.at(ProcInfoColumn::SystemTime) << " INTEGER NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::MonotonicTime) << " INTEGER NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::ReceiveTime) << " INTEGER NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::Comm) << " TEXT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::Pid) << " INTEGER NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::PPid) << " INTEGER NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::CtxId) << " TEXT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::CtxName) << " TEXT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::CpuTime) << " INTEGER NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::CpuPercent) << " INTEGER NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::MemRSS) << " INTEGER NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::MemPSS) << " INTEGER NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::FDCount) << " INTEGER NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::SessionId) << " INTEGER NOT NULL, ";
    } else {
      out << m_procInfoColumn.at(ProcInfoColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_procInfoColumn.at(ProcInfoColumn::SystemTime) << " BIGINT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::MonotonicTime) << " BIGINT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::ReceiveTime) << " BIGINT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::Comm) << " TEXT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::Pid) << " BIGINT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::PPid) << " BIGINT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::CtxId) << " TEXT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::CtxName) << " TEXT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::CpuTime) << " BIGINT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::CpuPercent) << " BIGINT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::MemRSS) << " BIGINT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::MemPSS) << " BIGINT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::FDCount) << " BIGINT NOT NULL, "
          << m_procInfoColumn.at(ProcInfoColumn::SessionId) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY(" << m_procInfoColumn.at(ProcInfoColumn::SessionId)
        << ") REFERENCES " << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // ContextInfo table
    out << "CREATE TABLE IF NOT EXISTS " << m_contextInfoTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_contextInfoColumn.at(ContextInfoColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_contextInfoColumn.at(ContextInfoColumn::SystemTime) << " INTEGER NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::MonotonicTime) << " INTEGER NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::ReceiveTime) << " INTEGER NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::CtxId) << " TEXT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::CtxName) << " TEXT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::TotalCpuTime) << " INTEGER NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::TotalCpuPercent) << " INTEGER NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::TotalMemRSS) << " INTEGER NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::TotalMemPSS) << " INTEGER NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::TotalFDCount) << " INTEGER NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::SessionId) << " INTEGER NOT NULL, ";
    } else {
      out << m_contextInfoColumn.at(ContextInfoColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_contextInfoColumn.at(ContextInfoColumn::SystemTime) << " BIGINT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::MonotonicTime) << " BIGINT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::ReceiveTime) << " BIGINT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::CtxId) << " TEXT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::CtxName) << " TEXT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::TotalCpuTime) << " BIGINT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::TotalCpuPercent) << " BIGINT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::TotalMemRSS) << " BIGINT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::TotalMemPSS) << " BIGINT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::TotalFDCount) << " BIGINT NOT NULL, "
          << m_contextInfoColumn.at(ContextInfoColumn::SessionId) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY("
        << m_contextInfoColumn.at(ContextInfoColumn::SessionId) << ") REFERENCES "
        << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";
  }

  // SysProcBuddyInfo table
  out << "CREATE TABLE IF NOT EXISTS " << m_sysProcBuddyInfoTableName << " (";
  if (type == Query::Type::SQLite3) {
    out << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::Id) << " INTEGER PRIMARY KEY, "
        << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::SystemTime) << " INTEGER NOT NULL, "
        << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::MonotonicTime)
        << " INTEGER NOT NULL, " << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::ReceiveTime)
        << " INTEGER NOT NULL, " << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::Name)
        << " TEXT NOT NULL, " << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::Zone)
        << " TEXT NOT NULL, " << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::Data)
        << " TEXT NOT NULL, " << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::SessionId)
        << " INTEGER NOT NULL, ";
  } else {
    out << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::Id) << " SERIAL PRIMARY KEY, "
        << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::SystemTime) << " BIGINT NOT NULL, "
        << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::MonotonicTime)
        << " BIGINT NOT NULL, " << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::ReceiveTime)
        << " BIGINT NOT NULL, " << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::Name)
        << " TEXT NOT NULL, " << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::Zone)
        << " TEXT NOT NULL, " << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::Data)
        << " TEXT NOT NULL, " << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::SessionId)
        << " INTEGER NOT NULL, ";
  }
  out << "CONSTRAINT KFSession FOREIGN KEY("
      << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::SessionId) << ") REFERENCES "
      << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
      << ") ON DELETE CASCADE);";

  // SysProcWireless table
  out << "CREATE TABLE IF NOT EXISTS " << m_sysProcWirelessTableName << " (";
  if (type == Query::Type::SQLite3) {
    out << m_sysProcWirelessColumn.at(SysProcWirelessColumn::Id) << " INTEGER PRIMARY KEY, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::SystemTime) << " INTEGER NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::MonotonicTime) << " INTEGER NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::ReceiveTime) << " INTEGER NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::Name) << " TEXT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::Status) << " TEXT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::QualityLink) << " INTEGER NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::QualityLevel) << " INTEGER NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::QualityNoise) << " INTEGER NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedNWId) << " INTEGER NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedCrypt)
        << " INTEGER NOT NULL, " << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedFrag)
        << " INTEGER NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedRetry)
        << " INTEGER NOT NULL, " << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedMisc)
        << " INTEGER NOT NULL, " << m_sysProcWirelessColumn.at(SysProcWirelessColumn::MissedBeacon)
        << " INTEGER NOT NULL, " << m_sysProcWirelessColumn.at(SysProcWirelessColumn::SessionId)
        << " INTEGER NOT NULL, ";
  } else {
    out << m_sysProcWirelessColumn.at(SysProcWirelessColumn::Id) << " SERIAL PRIMARY KEY, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::SystemTime) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::MonotonicTime) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::ReceiveTime) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::Name) << " TEXT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::Status) << " TEXT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::QualityLink) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::QualityLevel) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::QualityNoise) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedNWId) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedCrypt) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedFrag) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedRetry) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedMisc) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::MissedBeacon) << " BIGINT NOT NULL, "
        << m_sysProcWirelessColumn.at(SysProcWirelessColumn::SessionId) << " INTEGER NOT NULL, ";
  }
  out << "CONSTRAINT KFSession FOREIGN KEY("
      << m_sysProcWirelessColumn.at(SysProcWirelessColumn::SessionId) << ") REFERENCES "
      << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
      << ") ON DELETE CASCADE);";

  return out.str();
}

auto Query::dropTables(Query::Type type) -> std::string
{
  std::stringstream out;

  if (type == Query::Type::SQLite3) {
    out << "DROP TABLE IF EXISTS " << m_devicesTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sessionsTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sysProcStatTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sysProcMemInfoTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sysProcDiskStatsTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sysProcPressureTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sysProcBuddyInfoTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sysProcWirelessTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sysProcVMStatTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_procAcctTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_procInfoTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_procEventTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_contextInfoTableName << ";";
  } else if (type == Query::Type::PostgreSQL) {
    out << "DROP TABLE IF EXISTS " << m_devicesTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sessionsTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sysProcStatTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sysProcMemInfoTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sysProcDiskStatsTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sysProcPressureTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sysProcBuddyInfoTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sysProcWirelessTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sysProcVMStatTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_procAcctTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_procInfoTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_procEventTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_contextInfoTableName << " CASCADE;";
  }

  return out.str();
}

auto Query::getDevices(Query::Type type) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
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

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_devicesTableName << " (" << m_deviceColumn.at(DeviceColumn::Hash)
        << "," << m_deviceColumn.at(DeviceColumn::Name) << ","
        << m_deviceColumn.at(DeviceColumn::Address) << "," << m_deviceColumn.at(DeviceColumn::Port)
        << ") VALUES ("
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
  } else if (type == Query::Type::PostgreSQL) {
    out << "DELETE FROM " << m_devicesTableName << " WHERE "
        << m_deviceColumn.at(DeviceColumn::Hash) << " LIKE "
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
  } else if (type == Query::Type::PostgreSQL) {
    out << "SELECT * FROM " << m_devicesTableName << " WHERE "
        << m_deviceColumn.at(DeviceColumn::Hash) << " LIKE "
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
  } else if (type == Query::Type::PostgreSQL) {
    out << "SELECT " << m_deviceColumn.at(DeviceColumn::Id) << " FROM " << m_devicesTableName
        << " WHERE " << m_deviceColumn.at(DeviceColumn::Hash) << " LIKE "
        << "'" << hash << "' LIMIT 1;";
  }

  return out.str();
}

auto Query::getSessions(Query::Type type) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
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
  } else if (type == Query::Type::PostgreSQL) {
    out << "SELECT * FROM " << m_sessionsTableName << " WHERE "
        << m_sessionColumn.at(SessionColumn::Device) << " LIKE "
        << "(SELECT " << m_deviceColumn.at(DeviceColumn::Id) << " FROM " << m_devicesTableName
        << " WHERE " << m_deviceColumn.at(DeviceColumn::Hash) << " LIKE "
        << "'" << deviceHash << "');";
  }

  return out.str();
}

auto Query::addSession(Query::Type type,
                       const tkm::msg::monitor::SessionInfo &sessionInfo,
                       const std::string &deviceHash,
                       uint64_t startTimestamp) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_sessionsTableName << " (" << m_sessionColumn.at(SessionColumn::Hash)
        << "," << m_sessionColumn.at(SessionColumn::Name) << ","
        << m_sessionColumn.at(SessionColumn::CoreCount) << ","
        << m_sessionColumn.at(SessionColumn::StartTimestamp) << ","
        << m_sessionColumn.at(SessionColumn::EndTimestamp) << ","
        << m_sessionColumn.at(SessionColumn::Device) << ") VALUES ('" << sessionInfo.hash()
        << "', '" << sessionInfo.name() << "', '" << sessionInfo.core_count() << "', '"
        << startTimestamp << "', '"
        << "0"
        << "', ";
  }

  if (type == Query::Type::SQLite3) {
    out << "(SELECT " << m_deviceColumn.at(DeviceColumn::Id) << " FROM " << m_devicesTableName
        << " WHERE " << m_deviceColumn.at(DeviceColumn::Hash) << " IS "
        << "'" << deviceHash << "'));";
  } else if (type == Query::Type::PostgreSQL) {
    out << "(SELECT " << m_deviceColumn.at(DeviceColumn::Id) << " FROM " << m_devicesTableName
        << " WHERE " << m_deviceColumn.at(DeviceColumn::Hash) << " LIKE "
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
  } else if (type == Query::Type::PostgreSQL) {
    out << "UPDATE " << m_sessionsTableName << " SET "
        << m_sessionColumn.at(SessionColumn::EndTimestamp) << " = "
        << "'" << time(NULL) << "'"
        << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
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
  } else if (type == Query::Type::PostgreSQL) {
    out << "DELETE FROM " << m_sessionsTableName << " WHERE "
        << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
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
  } else if (type == Query::Type::PostgreSQL) {
    out << "SELECT * FROM " << m_sessionsTableName << " WHERE "
        << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
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
  } else if (type == Query::Type::PostgreSQL) {
    out << "SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
        << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
        << "'" << hash << "' LIMIT 1;";
  }

  return out.str();
}

auto Query::addData(Query::Type type,
                    const std::string &sessionHash,
                    const tkm::msg::monitor::ProcEvent &procEvent,
                    uint64_t systemTime,
                    uint64_t monotonicTime,
                    uint64_t receiveTime) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_procEventTableName << " ("
        << m_procEventColumn.at(ProcEventColumn::SystemTime) << ","
        << m_procEventColumn.at(ProcEventColumn::MonotonicTime) << ","
        << m_procEventColumn.at(ProcEventColumn::ReceiveTime) << ","
        << m_procEventColumn.at(ProcEventColumn::ForkCount) << ","
        << m_procEventColumn.at(ProcEventColumn::ExecCount) << ","
        << m_procEventColumn.at(ProcEventColumn::ExitCount) << ","
        << m_procEventColumn.at(ProcEventColumn::UIdCount) << ","
        << m_procEventColumn.at(ProcEventColumn::GIdCount) << ",";
    out << m_procEventColumn.at(ProcEventColumn::SessionId) << ") VALUES ('" << systemTime << "', '"
        << monotonicTime << "', '" << receiveTime << "', '" << procEvent.fork_count() << "', '"
        << procEvent.exec_count() << "', '" << procEvent.exit_count() << "', '"
        << procEvent.uid_count() << "', '" << procEvent.gid_count() << "', ";
  }

  if (type == Query::Type::SQLite3) {
    out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
        << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
        << "'" << sessionHash << "' AND EndTimestamp = 0));";
  } else {
    out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
        << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
        << "'" << sessionHash << "' AND EndTimestamp = 0));";
  }

  return out.str();
}

auto Query::addData(Query::Type type,
                    const std::string &sessionHash,
                    const tkm::msg::monitor::SysProcStat &sysProcStat,
                    uint64_t systemTime,
                    uint64_t monotonicTime,
                    uint64_t receiveTime) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_sysProcStatTableName << " ("
        << m_sysProcStatColumn.at(SysProcStatColumn::SystemTime) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::MonotonicTime) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::ReceiveTime) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatName) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatAll) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatUsr) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatSys) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatIow) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << ") VALUES ('" << systemTime
        << "', '" << monotonicTime << "', '" << receiveTime << "', '" << sysProcStat.cpu().name()
        << "', '" << sysProcStat.cpu().all() << "', '" << sysProcStat.cpu().usr() << "', '"
        << sysProcStat.cpu().sys() << "', '" << sysProcStat.cpu().iow() << "', ";

    if (type == Query::Type::SQLite3) {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    } else {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    }
  }

  for (const auto &cpuStat : sysProcStat.core()) {
    if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
      out << "INSERT INTO " << m_sysProcStatTableName << " ("
          << m_sysProcStatColumn.at(SysProcStatColumn::SystemTime) << ","
          << m_sysProcStatColumn.at(SysProcStatColumn::MonotonicTime) << ","
          << m_sysProcStatColumn.at(SysProcStatColumn::ReceiveTime) << ","
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatName) << ","
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatAll) << ","
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatUsr) << ","
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatSys) << ","
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatIow) << ","
          << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << ") VALUES ('" << systemTime
          << "', '" << monotonicTime << "', '" << receiveTime << "', '" << cpuStat.name() << "', '"
          << cpuStat.all() << "', '" << cpuStat.usr() << "', '" << cpuStat.sys() << "', '"
          << cpuStat.iow() << "', ";

      if (type == Query::Type::SQLite3) {
        out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM "
            << m_sessionsTableName << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
            << "'" << sessionHash << "' AND EndTimestamp = 0));";
      } else {
        out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM "
            << m_sessionsTableName << " WHERE " << m_sessionColumn.at(SessionColumn::Hash)
            << " LIKE "
            << "'" << sessionHash << "' AND EndTimestamp = 0));";
      }
    }
  }

  return out.str();
}

auto Query::addData(Query::Type type,
                    const std::string &sessionHash,
                    const tkm::msg::monitor::SysProcMemInfo &sysProcMem,
                    uint64_t systemTime,
                    uint64_t monotonicTime,
                    uint64_t receiveTime) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_sysProcMemInfoTableName << " ("
        << m_sysProcMemColumn.at(SysProcMemColumn::SystemTime) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::MonotonicTime) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::ReceiveTime) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::MemTotal) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::MemFree) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::MemAvail) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::MemCached) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::MemAvailPercent) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::Active) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::Inactive) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::Slab) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::KReclaimable) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SReclaimable) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SUnreclaim) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::KernelStack) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SwapTotal) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SwapFree) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SwapCached) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SwapFreePercent) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::CmaTotal) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::CmaFree) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SessionId) << ") VALUES ('" << systemTime
        << "', '" << monotonicTime << "', '" << receiveTime << "', '" << sysProcMem.mem_total()
        << "', '" << sysProcMem.mem_free() << "', '" << sysProcMem.mem_available() << "', '"
        << sysProcMem.mem_cached() << "', '" << sysProcMem.mem_percent() << "', '"
        << sysProcMem.active() << "', '" << sysProcMem.inactive() << "', '"
        << sysProcMem.slab() << "', '" << sysProcMem.kreclaimable() << "', '"
        << sysProcMem.sreclaimable() << "', '" << sysProcMem.sunreclaim() << "', '"
        << sysProcMem.kernel_stack() << "', '"
        << sysProcMem.swap_total() << "', '" << sysProcMem.swap_free() << "', '"
        << sysProcMem.swap_cached() << "', '" << sysProcMem.swap_percent() << "', '"
        << sysProcMem.cma_total() << "', '" << sysProcMem.cma_free() << "', ";

    if (type == Query::Type::SQLite3) {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    } else {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    }
  }

  return out.str();
}

auto Query::addData(Query::Type type,
                    const std::string &sessionHash,
                    const tkm::msg::monitor::SysProcDiskStats &sysDiskStats,
                    uint64_t systemTime,
                    uint64_t monotonicTime,
                    uint64_t receiveTime) -> std::string
{
  std::stringstream out;

  for (const auto &diskEntry : sysDiskStats.disk()) {
    if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
      out << "INSERT INTO " << m_sysProcDiskStatsTableName << " ("
          << m_sysProcDiskColumn.at(SysProcDiskColumn::SystemTime) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::MonotonicTime) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReceiveTime) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::Major) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::Minor) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::Name) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReadsCompleted) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReadsMerged) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::ReadsSpentMs) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::WritesCompleted) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::WritesMerged) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::WritesSpentMs) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::IOInProgress) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::IOSpentMs) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::IOWeightedMs) << ","
          << m_sysProcDiskColumn.at(SysProcDiskColumn::SessionId) << ") VALUES ('" << systemTime
          << "', '" << monotonicTime << "', '" << receiveTime << "', '" << diskEntry.node_major()
          << "', '" << diskEntry.node_minor() << "', '" << diskEntry.name() << "', '"
          << diskEntry.reads_completed() << "', '" << diskEntry.reads_merged() << "', '"
          << diskEntry.reads_spent_ms() << "', '" << diskEntry.writes_completed() << "', '"
          << diskEntry.writes_merged() << "', '" << diskEntry.writes_spent_ms() << "', '"
          << diskEntry.io_in_progress() << "', '" << diskEntry.io_spent_ms() << "', '"
          << diskEntry.io_weighted_ms() << "', ";

      if (type == Query::Type::SQLite3) {
        out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM "
            << m_sessionsTableName << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
            << "'" << sessionHash << "' AND EndTimestamp = 0));";
      } else {
        out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM "
            << m_sessionsTableName << " WHERE " << m_sessionColumn.at(SessionColumn::Hash)
            << " LIKE "
            << "'" << sessionHash << "' AND EndTimestamp = 0));";
      }
    }
  }

  return out.str();
}

auto Query::addData(Query::Type type,
                    const std::string &sessionHash,
                    const tkm::msg::monitor::SysProcPressure &sysProcPressure,
                    uint64_t systemTime,
                    uint64_t monotonicTime,
                    uint64_t receiveTime) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_sysProcPressureTableName << " ("
        << m_sysProcPressureColumn.at(SysProcPressureColumn::SystemTime) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::MonotonicTime) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::ReceiveTime) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg10) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg60) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg300) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeTotal) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg10) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg60) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg300) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullTotal) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg10) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg60) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg300) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeTotal) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg10) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg60) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg300) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullTotal) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg10) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg60) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg300) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeTotal) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg10) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg60) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg300) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullTotal) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << ") VALUES ('" << systemTime
        << "', '" << monotonicTime << "', '" << receiveTime << "', '"
        << sysProcPressure.cpu_some().avg10() << "', '" << sysProcPressure.cpu_some().avg60()
        << "', '" << sysProcPressure.cpu_some().avg300() << "', '"
        << sysProcPressure.cpu_some().total() << "', '" << sysProcPressure.cpu_full().avg10()
        << "', '" << sysProcPressure.cpu_full().avg60() << "', '"
        << sysProcPressure.cpu_full().avg300() << "', '" << sysProcPressure.cpu_full().total()
        << "', '" << sysProcPressure.mem_some().avg10() << "', '"
        << sysProcPressure.mem_some().avg60() << "', '" << sysProcPressure.mem_some().avg300()
        << "', '" << sysProcPressure.mem_some().total() << "', '"
        << sysProcPressure.mem_full().avg10() << "', '" << sysProcPressure.mem_full().avg60()
        << "', '" << sysProcPressure.mem_full().avg300() << "', '"
        << sysProcPressure.mem_full().total() << "', '" << sysProcPressure.io_some().avg10()
        << "', '" << sysProcPressure.io_some().avg60() << "', '"
        << sysProcPressure.io_some().avg300() << "', '" << sysProcPressure.io_some().total()
        << "', '" << sysProcPressure.io_full().avg10() << "', '"
        << sysProcPressure.io_full().avg60() << "', '" << sysProcPressure.io_full().avg300()
        << "', '" << sysProcPressure.io_full().total() << "', ";

    if (type == Query::Type::SQLite3) {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    } else {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    }
  }

  return out.str();
}

auto Query::addData(Query::Type type,
                    const std::string &sessionHash,
                    const tkm::msg::monitor::ProcAcct &procAcct,
                    uint64_t systemTime,
                    uint64_t monotonicTime,
                    uint64_t receiveTime) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_procAcctTableName << " ("
        << m_procAcctColumn.at(ProcAcctColumn::SystemTime) << ","
        << m_procAcctColumn.at(ProcAcctColumn::MonotonicTime) << ","
        << m_procAcctColumn.at(ProcAcctColumn::ReceiveTime) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcComm) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcUid) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcGid) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcPid) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcPPid) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcUTime) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcSTime) << ","
        << m_procAcctColumn.at(ProcAcctColumn::CpuCount) << ","
        << m_procAcctColumn.at(ProcAcctColumn::CpuRunRealTotal) << ","
        << m_procAcctColumn.at(ProcAcctColumn::CpuRunVirtualTotal) << ","
        << m_procAcctColumn.at(ProcAcctColumn::CpuDelayTotal) << ","
        << m_procAcctColumn.at(ProcAcctColumn::CpuDelayAverage) << ","
        << m_procAcctColumn.at(ProcAcctColumn::CoreMem) << ","
        << m_procAcctColumn.at(ProcAcctColumn::VirtMem) << ","
        << m_procAcctColumn.at(ProcAcctColumn::HiwaterRss) << ","
        << m_procAcctColumn.at(ProcAcctColumn::HiwaterVm) << ","
        << m_procAcctColumn.at(ProcAcctColumn::Nvcsw) << ","
        << m_procAcctColumn.at(ProcAcctColumn::Nivcsw) << ","
        << m_procAcctColumn.at(ProcAcctColumn::SwapinCount) << ","
        << m_procAcctColumn.at(ProcAcctColumn::SwapinDelayTotal) << ","
        << m_procAcctColumn.at(ProcAcctColumn::SwapinDelayAverage) << ","
        << m_procAcctColumn.at(ProcAcctColumn::BlkIOCount) << ","
        << m_procAcctColumn.at(ProcAcctColumn::BlkIODelayTotal) << ","
        << m_procAcctColumn.at(ProcAcctColumn::BlkIODelayAverage) << ","
        << m_procAcctColumn.at(ProcAcctColumn::IOStorageReadBytes) << ","
        << m_procAcctColumn.at(ProcAcctColumn::IOStorageWriteBytes) << ","
        << m_procAcctColumn.at(ProcAcctColumn::IOReadChar) << ","
        << m_procAcctColumn.at(ProcAcctColumn::IOWriteChar) << ","
        << m_procAcctColumn.at(ProcAcctColumn::IOReadSyscalls) << ","
        << m_procAcctColumn.at(ProcAcctColumn::IOWriteSyscalls) << ","
        << m_procAcctColumn.at(ProcAcctColumn::FreePagesCount) << ","
        << m_procAcctColumn.at(ProcAcctColumn::FreePagesDelayTotal) << ","
        << m_procAcctColumn.at(ProcAcctColumn::FreePagesDelayAverage) << ","
        << m_procAcctColumn.at(ProcAcctColumn::ThrashingCount) << ","
        << m_procAcctColumn.at(ProcAcctColumn::ThrashingDelayTotal) << ","
        << m_procAcctColumn.at(ProcAcctColumn::ThrashingDelayAverage) << ","
        << m_procAcctColumn.at(ProcAcctColumn::SessionId) << ") VALUES ('" << systemTime << "', '"
        << monotonicTime << "', '" << receiveTime << "', '" << procAcct.ac_comm() << "', '"
        << procAcct.ac_uid() << "', '" << procAcct.ac_gid() << "', '" << procAcct.ac_pid() << "', '"
        << procAcct.ac_ppid() << "', '" << procAcct.ac_utime() << "', '" << procAcct.ac_stime()
        << "', '" << procAcct.cpu().cpu_count() << "', '" << procAcct.cpu().cpu_run_real_total()
        << "', '" << procAcct.cpu().cpu_run_virtual_total() << "', '"
        << procAcct.cpu().cpu_delay_total() << "', '" << procAcct.cpu().cpu_delay_average()
        << "', '" << procAcct.mem().coremem() << "', '" << procAcct.mem().virtmem() << "', '"
        << procAcct.mem().hiwater_rss() << "', '" << procAcct.mem().hiwater_vm() << "', '"
        << procAcct.ctx().nvcsw() << "', '" << procAcct.ctx().nivcsw() << "', '"
        << procAcct.swp().swapin_count() << "', '" << procAcct.swp().swapin_delay_total() << "', '"
        << procAcct.swp().swapin_delay_average() << "', '" << procAcct.io().blkio_count() << "', '"
        << procAcct.io().blkio_delay_total() << "', '" << procAcct.io().blkio_delay_average()
        << "', '" << procAcct.io().read_bytes() << "', '" << procAcct.io().write_bytes() << "', '"
        << procAcct.io().read_char() << "', '" << procAcct.io().write_char() << "', '"
        << procAcct.io().read_syscalls() << "', '" << procAcct.io().write_syscalls() << "', '"
        << procAcct.reclaim().freepages_count() << "', '"
        << procAcct.reclaim().freepages_delay_total() << "', '"
        << procAcct.reclaim().freepages_delay_average() << "', '"
        << procAcct.thrashing().thrashing_count() << "', '"
        << procAcct.thrashing().thrashing_delay_total() << "', '"
        << procAcct.thrashing().thrashing_delay_average() << "', ";

    if (type == Query::Type::SQLite3) {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    } else {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    }
  }

  return out.str();
}

auto Query::addData(Query::Type type,
                    const std::string &sessionHash,
                    const tkm::msg::monitor::ProcInfo &procInfo,
                    uint64_t systemTime,
                    uint64_t monotonicTime,
                    uint64_t receiveTime) -> std::string
{
  std::stringstream out;

  for (const auto &procEntry : procInfo.entry()) {
    if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
      out << "INSERT INTO " << m_procInfoTableName << " ("
          << m_procInfoColumn.at(ProcInfoColumn::SystemTime) << ","
          << m_procInfoColumn.at(ProcInfoColumn::MonotonicTime) << ","
          << m_procInfoColumn.at(ProcInfoColumn::ReceiveTime) << ","
          << m_procInfoColumn.at(ProcInfoColumn::Comm) << ","
          << m_procInfoColumn.at(ProcInfoColumn::Pid) << ","
          << m_procInfoColumn.at(ProcInfoColumn::PPid) << ","
          << m_procInfoColumn.at(ProcInfoColumn::CtxId) << ","
          << m_procInfoColumn.at(ProcInfoColumn::CtxName) << ","
          << m_procInfoColumn.at(ProcInfoColumn::CpuTime) << ","
          << m_procInfoColumn.at(ProcInfoColumn::CpuPercent) << ","
          << m_procInfoColumn.at(ProcInfoColumn::MemRSS) << ","
          << m_procInfoColumn.at(ProcInfoColumn::MemPSS) << ","
          << m_procInfoColumn.at(ProcInfoColumn::FDCount) << ","
          << m_procInfoColumn.at(ProcInfoColumn::SessionId) << ") VALUES ('" << systemTime << "', '"
          << monotonicTime << "', '" << receiveTime << "', '" << procEntry.comm() << "', '"
          << procEntry.pid() << "', '" << procEntry.ppid() << "', '"
          << std::to_string(procEntry.ctx_id()) << "', '" << procEntry.ctx_name() << "', '"
          << procEntry.cpu_time() << "', '" << procEntry.cpu_percent() << "', '"
          << procEntry.mem_rss() << "', '" << procEntry.mem_pss() << "', '"
          << procEntry.fd_count() << "', ";
    }

    if (type == Query::Type::SQLite3) {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    } else {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    }
  }

  return out.str();
}

auto Query::addData(Query::Type type,
                    const std::string &sessionHash,
                    const tkm::msg::monitor::ContextInfo &ctxInfo,
                    uint64_t systemTime,
                    uint64_t monotonicTime,
                    uint64_t receiveTime) -> std::string
{
  std::stringstream out;

  for (const auto &ctxEntry : ctxInfo.entry()) {
    if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
      out << "INSERT INTO " << m_contextInfoTableName << " ("
          << m_contextInfoColumn.at(ContextInfoColumn::SystemTime) << ","
          << m_contextInfoColumn.at(ContextInfoColumn::MonotonicTime) << ","
          << m_contextInfoColumn.at(ContextInfoColumn::ReceiveTime) << ","
          << m_contextInfoColumn.at(ContextInfoColumn::CtxId) << ","
          << m_contextInfoColumn.at(ContextInfoColumn::CtxName) << ","
          << m_contextInfoColumn.at(ContextInfoColumn::TotalCpuTime) << ","
          << m_contextInfoColumn.at(ContextInfoColumn::TotalCpuPercent) << ","
          << m_contextInfoColumn.at(ContextInfoColumn::TotalMemRSS) << ","
          << m_contextInfoColumn.at(ContextInfoColumn::TotalMemPSS) << ","
          << m_contextInfoColumn.at(ContextInfoColumn::TotalFDCount) << ","
          << m_contextInfoColumn.at(ContextInfoColumn::SessionId) << ") VALUES ('" << systemTime
          << "', '" << monotonicTime << "', '" << receiveTime << "', '"
          << std::to_string(ctxEntry.ctx_id()) << "', '" << ctxEntry.ctx_name() << "', '"
          << ctxEntry.total_cpu_time() << "', '" << ctxEntry.total_cpu_percent() << "', '"
          << ctxEntry.total_mem_rss() << "', '" << ctxEntry.total_mem_pss() << "', '"
          << ctxEntry.total_fd_count() << "', ";
    }

    if (type == Query::Type::SQLite3) {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    } else {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    }
  }

  return out.str();
}

auto Query::addData(Query::Type type,
                    const std::string &sessionHash,
                    const tkm::msg::monitor::SysProcBuddyInfo &sysProcBuddyInfo,
                    uint64_t systemTime,
                    uint64_t monotonicTime,
                    uint64_t receiveTime) -> std::string
{
  std::stringstream out;

  for (const auto &buddyInfo : sysProcBuddyInfo.node()) {
    if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
      out << "INSERT INTO " << m_sysProcBuddyInfoTableName << " ("
          << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::SystemTime) << ","
          << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::MonotonicTime) << ","
          << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::ReceiveTime) << ","
          << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::Name) << ","
          << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::Zone) << ","
          << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::Data) << ","
          << m_sysProcBuddyInfoColumn.at(SysProcBuddyInfoColumn::SessionId) << ") VALUES ('"
          << systemTime << "', '" << monotonicTime << "', '" << receiveTime << "', '"
          << buddyInfo.name() << "', '" << buddyInfo.zone() << "', '" << buddyInfo.data() << "', ";

      if (type == Query::Type::SQLite3) {
        out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM "
            << m_sessionsTableName << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
            << "'" << sessionHash << "' AND EndTimestamp = 0));";
      } else {
        out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM "
            << m_sessionsTableName << " WHERE " << m_sessionColumn.at(SessionColumn::Hash)
            << " LIKE "
            << "'" << sessionHash << "' AND EndTimestamp = 0));";
      }
    }
  }

  return out.str();
}

auto Query::addData(Query::Type type,
                    const std::string &sessionHash,
                    const tkm::msg::monitor::SysProcWireless &sysProcWireless,
                    uint64_t systemTime,
                    uint64_t monotonicTime,
                    uint64_t receiveTime) -> std::string
{
  std::stringstream out;

  for (const auto &ifw : sysProcWireless.ifw()) {
    if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
      out << "INSERT INTO " << m_sysProcWirelessTableName << " ("
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::SystemTime) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::MonotonicTime) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::ReceiveTime) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::Name) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::Status) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::QualityLink) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::QualityLevel) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::QualityNoise) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedNWId) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedCrypt) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedFrag) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedRetry) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::DiscardedMisc) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::MissedBeacon) << ","
          << m_sysProcWirelessColumn.at(SysProcWirelessColumn::SessionId) << ") VALUES ('"
          << systemTime << "', '" << monotonicTime << "', '" << receiveTime << "', '" << ifw.name()
          << "', '" << ifw.status() << "', '" << ifw.quality_link() << "', '" << ifw.quality_level()
          << "', '" << ifw.quality_noise() << "', '" << ifw.discarded_nwid() << "', '"
          << ifw.discarded_crypt() << "', '" << ifw.discarded_frag() << "', '"
          << ifw.discarded_retry() << "', '" << ifw.discarded_misc() << "', '"
          << ifw.missed_beacon() << "', ";

      if (type == Query::Type::SQLite3) {
        out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM "
            << m_sessionsTableName << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
            << "'" << sessionHash << "' AND EndTimestamp = 0));";
      } else {
        out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM "
            << m_sessionsTableName << " WHERE " << m_sessionColumn.at(SessionColumn::Hash)
            << " LIKE "
            << "'" << sessionHash << "' AND EndTimestamp = 0));";
      }
    }
  }

  return out.str();
}

auto Query::addData(Query::Type type,
                    const std::string &sessionHash,
                    const tkm::msg::monitor::SysProcVMStat &sysProcVMStat,
                    uint64_t systemTime,
                    uint64_t monotonicTime,
                    uint64_t receiveTime) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_sysProcVMStatTableName << " ("
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::SystemTime) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::MonotonicTime) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ReceiveTime) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGpgin) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGpgout) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PSwpin) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PSwpout) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGmajfault) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGreuse) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealKswapd) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealDirect) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealKhugepaged) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealAnon) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGStealFile) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanKswapd) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanDirect) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanKhugepaged) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanDirectThrottle) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanAnon) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::PGScanFile) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::OOMKill) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::CompactStall) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::CompactFail) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::CompactSuccess) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpFaultAlloc) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpCollapseAlloc) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpCollapseAllocFailed) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpFileAlloc) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpFileMapped) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSplitPage) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSplitPageFailed) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpZeroPageAlloc) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpZeroPageAllocFailed) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSwpout) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::ThpSwpoutFallback) << ","
        << m_sysProcVMStatColumn.at(SysProcVMStatColumn::SessionId) << ") VALUES ('" << systemTime
        << "', '" << monotonicTime << "', '" << receiveTime << "', '" << sysProcVMStat.pgpgin()
        << "', '" << sysProcVMStat.pgpgout() << "', '" << sysProcVMStat.pswpin() << "', '"
        << sysProcVMStat.pswpout() << "', '" << sysProcVMStat.pgmajfault() << "', '"
        << sysProcVMStat.pgreuse() << "', '" << sysProcVMStat.pgsteal_kswapd() << "', '"
        << sysProcVMStat.pgsteal_direct() << "', '" << sysProcVMStat.pgsteal_khugepaged() << "', '"
        << sysProcVMStat.pgsteal_anon() << "', '" << sysProcVMStat.pgsteal_file() << "', '"
        << sysProcVMStat.pgscan_kswapd() << "', '"
        << sysProcVMStat.pgscan_direct() << "', '" << sysProcVMStat.pgscan_khugepaged() << "', '"
        << sysProcVMStat.pgscan_direct_throttle() << "', '" << sysProcVMStat.pgscan_anon() << "', '"
        << sysProcVMStat.pgscan_file() << "', '" << sysProcVMStat.oom_kill() << "', '"
        << sysProcVMStat.compact_stall() << "', '" << sysProcVMStat.compact_fail() << "', '"
        << sysProcVMStat.compact_success() << "', '" << sysProcVMStat.thp_fault_alloc() << "', '"
        << sysProcVMStat.thp_collapse_alloc() << "', '" << sysProcVMStat.thp_collapse_alloc_failed() << "', '"
        << sysProcVMStat.thp_file_alloc() << "', '" << sysProcVMStat.thp_file_mapped() << "', '"
        << sysProcVMStat.thp_split_page() << "', '" << sysProcVMStat.thp_split_page_failed() << "', '"
        << sysProcVMStat.thp_zero_page_alloc() << "', '" << sysProcVMStat.thp_zero_page_alloc_failed() << "', '"
        << sysProcVMStat.thp_swpout() << "', '" << sysProcVMStat.thp_swpout_fallback() << "', ";

    if (type == Query::Type::SQLite3) {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " IS "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    } else {
      out << "(SELECT " << m_sessionColumn.at(SessionColumn::Id) << " FROM " << m_sessionsTableName
          << " WHERE " << m_sessionColumn.at(SessionColumn::Hash) << " LIKE "
          << "'" << sessionHash << "' AND EndTimestamp = 0));";
    }
  }

  return out.str();
}
} // namespace tkm

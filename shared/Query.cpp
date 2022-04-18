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
#include "Server.pb.h"
#include <string>

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
          << m_sessionColumn.at(SessionColumn::StartTimestamp) << " INTEGER NOT NULL, "
          << m_sessionColumn.at(SessionColumn::EndTimestamp) << " INTEGER NOT NULL, "
          << m_sessionColumn.at(SessionColumn::Device) << " INTEGER NOT NULL, ";
    } else {
      out << m_sessionColumn.at(SessionColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_sessionColumn.at(SessionColumn::Name) << " TEXT NOT NULL, "
          << m_sessionColumn.at(SessionColumn::Hash) << " TEXT NOT NULL, "
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
          << m_procEventColumn.at(ProcEventColumn::Timestamp) << " INTEGER NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::RecvTime) << " INTEGER NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::What) << " TEXT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::ProcessPID) << " INTEGER, "
          << m_procEventColumn.at(ProcEventColumn::ProcessTGID) << " INTEGER, "
          << m_procEventColumn.at(ProcEventColumn::ParentPID) << " INTEGER, "
          << m_procEventColumn.at(ProcEventColumn::ParentTGID) << " INTEGER, "
          << m_procEventColumn.at(ProcEventColumn::ChildPID) << " INTEGER, "
          << m_procEventColumn.at(ProcEventColumn::ChildTGID) << " INTEGER, "
          << m_procEventColumn.at(ProcEventColumn::ExitCode) << " INTEGER, "
          << m_procEventColumn.at(ProcEventColumn::ProcessRID) << " INTEGER, "
          << m_procEventColumn.at(ProcEventColumn::ProcessEID) << " INTEGER, "
          << m_procEventColumn.at(ProcEventColumn::SessionId) << " INTEGER NOT NULL, ";
    } else {
      out << m_procEventColumn.at(ProcEventColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_procEventColumn.at(ProcEventColumn::Timestamp) << " BIGINT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::RecvTime) << " BIGINT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::What) << " TEXT NOT NULL, "
          << m_procEventColumn.at(ProcEventColumn::ProcessPID) << " BIGINT, "
          << m_procEventColumn.at(ProcEventColumn::ProcessTGID) << " BIGINT, "
          << m_procEventColumn.at(ProcEventColumn::ParentPID) << " BIGINT, "
          << m_procEventColumn.at(ProcEventColumn::ParentTGID) << " BIGINT, "
          << m_procEventColumn.at(ProcEventColumn::ChildPID) << " BIGINT, "
          << m_procEventColumn.at(ProcEventColumn::ChildTGID) << " BIGINT, "
          << m_procEventColumn.at(ProcEventColumn::ExitCode) << " BIGINT, "
          << m_procEventColumn.at(ProcEventColumn::ProcessRID) << " BIGINT, "
          << m_procEventColumn.at(ProcEventColumn::ProcessEID) << " BIGINT, "
          << m_procEventColumn.at(ProcEventColumn::SessionId) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY(" << m_procEventColumn.at(ProcEventColumn::SessionId)
        << ") REFERENCES " << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // SysProcStat table
    out << "CREATE TABLE IF NOT EXISTS " << m_sysProcStatTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_sysProcStatColumn.at(SysProcStatColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_sysProcStatColumn.at(SysProcStatColumn::Timestamp) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::RecvTime) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatName) << " TEXT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatAll) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatUsr) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatSys) << " INTEGER NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << " INTEGER NOT NULL, ";
    } else {
      out << m_sysProcStatColumn.at(SysProcStatColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_sysProcStatColumn.at(SysProcStatColumn::Timestamp) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::RecvTime) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatName) << " TEXT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatAll) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatUsr) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatSys) << " BIGINT NOT NULL, "
          << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY("
        << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << ") REFERENCES "
        << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // SysProcMeminfo table
    out << "CREATE TABLE IF NOT EXISTS " << m_sysProcMeminfoTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_sysProcMemColumn.at(SysProcMemColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_sysProcMemColumn.at(SysProcMemColumn::Timestamp) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::RecvTime) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemTotal) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemFree) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemAvail) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemCached) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemAvailPercent) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapTotal) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapFree) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapCached) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapFreePercent) << " INTEGER NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SessionId) << " INTEGER NOT NULL, ";
    } else {
      out << m_sysProcMemColumn.at(SysProcMemColumn::Id) << " SERIAL PRIMARY KEY, "
          << m_sysProcMemColumn.at(SysProcMemColumn::Timestamp) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::RecvTime) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemTotal) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemFree) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemAvail) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemCached) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::MemAvailPercent) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapTotal) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapFree) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapCached) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SwapFreePercent) << " BIGINT NOT NULL, "
          << m_sysProcMemColumn.at(SysProcMemColumn::SessionId) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY(" << m_sysProcMemColumn.at(SysProcMemColumn::SessionId)
        << ") REFERENCES " << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // SysProcPressure table
    out << "CREATE TABLE IF NOT EXISTS " << m_sysProcPressureTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_sysProcPressureColumn.at(SysProcPressureColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::Timestamp) << " INTEGER NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::RecvTime) << " INTEGER NOT NULL, "
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
          << m_sysProcPressureColumn.at(SysProcPressureColumn::Timestamp) << " BIGINT NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::RecvTime) << " BIGINT NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg10) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg60) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeAvg300) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUSomeTotal) << " BIGINT NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg10) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg60) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullAvg300) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::CPUFullTotal) << " BIGINT NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg10) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg60) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeAvg300) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMSomeTotal) << " BIGINT NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg10) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg60) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullAvg300) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::MEMFullTotal) << " BIGINT NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg10) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg60) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeAvg300) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::IOSomeTotal) << " BIGINT NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg10) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg60) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullAvg300) << " REAL NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::IOFullTotal) << " BIGINT NOT NULL, "
          << m_sysProcPressureColumn.at(SysProcPressureColumn::SessionId) << " INTEGER NOT NULL, ";
    }
    out << "CONSTRAINT KFSession FOREIGN KEY("
        << m_sysProcPressureColumn.at(SysProcPressureColumn::SessionId) << ") REFERENCES "
        << m_sessionsTableName << "(" << m_sessionColumn.at(SessionColumn::Id)
        << ") ON DELETE CASCADE);";

    // ProcAcct table
    out << "CREATE TABLE IF NOT EXISTS " << m_procAcctTableName << " (";
    if (type == Query::Type::SQLite3) {
      out << m_procAcctColumn.at(ProcAcctColumn::Id) << " INTEGER PRIMARY KEY, "
          << m_procAcctColumn.at(ProcAcctColumn::Timestamp) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::RecvTime) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcComm) << " TEXT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcUid) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcGid) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcPid) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcPPid) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcUTime) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcSTime) << " INTEGER NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuPercent) << " INTEGER NOT NULL, "
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
          << m_procAcctColumn.at(ProcAcctColumn::Timestamp) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::RecvTime) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcComm) << " TEXT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcUid) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcGid) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcPid) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcPPid) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcUTime) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::AcSTime) << " BIGINT NOT NULL, "
          << m_procAcctColumn.at(ProcAcctColumn::CpuPercent) << " INTEGER NOT NULL, "
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
  }

  return out.str();
}

auto Query::dropTables(Query::Type type) -> std::string
{
  std::stringstream out;

  if (type == Query::Type::SQLite3) {
    out << "DROP TABLE IF EXISTS " << m_devicesTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sessionsTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sysProcStatTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sysProcMeminfoTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_sysProcPressureTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_procAcctTableName << ";";
    out << "DROP TABLE IF EXISTS " << m_procEventTableName << ";";
  } else if (type == Query::Type::PostgreSQL) {
    out << "DROP TABLE IF EXISTS " << m_devicesTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sessionsTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sysProcStatTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sysProcMeminfoTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_sysProcPressureTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_procAcctTableName << " CASCADE;";
    out << "DROP TABLE IF EXISTS " << m_procEventTableName << " CASCADE;";
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
                       const std::string &hash,
                       const std::string &name,
                       uint64_t startTimestamp,
                       const std::string &deviceHash) -> std::string
{
  std::stringstream out;

  if (type == Query::Type::SQLite3) {
    out << "INSERT INTO " << m_sessionsTableName << " (" << m_sessionColumn.at(SessionColumn::Hash)
        << "," << m_sessionColumn.at(SessionColumn::Name) << ","
        << m_sessionColumn.at(SessionColumn::StartTimestamp) << ","
        << m_sessionColumn.at(SessionColumn::EndTimestamp) << ","
        << m_sessionColumn.at(SessionColumn::Device) << ") VALUES ('" << hash << "', '" << name
        << "', '" << startTimestamp << "', '"
        << "0"
        << "', "
        << "(SELECT " << m_deviceColumn.at(DeviceColumn::Id) << " FROM " << m_devicesTableName
        << " WHERE " << m_deviceColumn.at(DeviceColumn::Hash) << " IS "
        << "'" << deviceHash << "'));";
  } else if (type == Query::Type::PostgreSQL) {
    out << "INSERT INTO " << m_sessionsTableName << " (" << m_sessionColumn.at(SessionColumn::Hash)
        << "," << m_sessionColumn.at(SessionColumn::Name) << ","
        << m_sessionColumn.at(SessionColumn::StartTimestamp) << ","
        << m_sessionColumn.at(SessionColumn::EndTimestamp) << ","
        << m_sessionColumn.at(SessionColumn::Device) << ") VALUES ('" << hash << "', '" << name
        << "', '" << startTimestamp << "', '"
        << "0"
        << "', "
        << "(SELECT " << m_deviceColumn.at(DeviceColumn::Id) << " FROM " << m_devicesTableName
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
                    const tkm::msg::server::ProcEvent &procEvent,
                    uint64_t ts,
                    uint64_t recvTime) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_procEventTableName << " ("
        << m_procEventColumn.at(ProcEventColumn::Timestamp) << ","
        << m_procEventColumn.at(ProcEventColumn::RecvTime) << ","
        << m_procEventColumn.at(ProcEventColumn::What) << ",";

    switch (procEvent.what()) {
    case tkm::msg::server::ProcEvent::What::ProcEvent_What_Fork:
      out << m_procEventColumn.at(ProcEventColumn::ParentPID) << ",";
      out << m_procEventColumn.at(ProcEventColumn::ParentTGID) << ",";
      out << m_procEventColumn.at(ProcEventColumn::ChildPID) << ",";
      out << m_procEventColumn.at(ProcEventColumn::ChildTGID) << ",";
      break;
    case tkm::msg::server::ProcEvent::What::ProcEvent_What_Exec:
      out << m_procEventColumn.at(ProcEventColumn::ProcessPID) << ",";
      out << m_procEventColumn.at(ProcEventColumn::ProcessTGID) << ",";
      break;
    case tkm::msg::server::ProcEvent::What::ProcEvent_What_Exit:
      out << m_procEventColumn.at(ProcEventColumn::ProcessPID) << ",";
      out << m_procEventColumn.at(ProcEventColumn::ProcessTGID) << ",";
      out << m_procEventColumn.at(ProcEventColumn::ExitCode) << ",";
      break;
    case tkm::msg::server::ProcEvent::What::ProcEvent_What_UID:
    case tkm::msg::server::ProcEvent::What::ProcEvent_What_GID:
      out << m_procEventColumn.at(ProcEventColumn::ProcessPID) << ",";
      out << m_procEventColumn.at(ProcEventColumn::ProcessTGID) << ",";
      out << m_procEventColumn.at(ProcEventColumn::ProcessRID) << ",";
      out << m_procEventColumn.at(ProcEventColumn::ProcessEID) << ",";
      break;
    default:
      break;
    }

    out << m_procEventColumn.at(ProcEventColumn::SessionId) << ") VALUES ('" << ts << "', '"
        << recvTime << "', '";

    switch (procEvent.what()) {
    case tkm::msg::server::ProcEvent::What::ProcEvent_What_Fork: {
      tkm::msg::server::ProcEventFork data;
      procEvent.data().UnpackTo(&data);
      out << "fork"
          << "', '";
      out << data.parent_pid() << "', '" << data.parent_tgid() << "', '" << data.child_pid()
          << "', '" << data.child_tgid() << "', ";
      break;
    }
    case tkm::msg::server::ProcEvent::What::ProcEvent_What_Exec: {
      tkm::msg::server::ProcEventExec data;
      procEvent.data().UnpackTo(&data);
      out << "exec"
          << "', '";
      out << data.process_pid() << "', '" << data.process_tgid() << "', ";
      break;
    }
    case tkm::msg::server::ProcEvent::What::ProcEvent_What_Exit: {
      tkm::msg::server::ProcEventExit data;
      procEvent.data().UnpackTo(&data);
      out << "exit"
          << "', '";
      out << data.process_pid() << "', '" << data.process_tgid() << "', '" << data.exit_code()
          << "', ";
      break;
    }
    case tkm::msg::server::ProcEvent::What::ProcEvent_What_UID: {
      tkm::msg::server::ProcEventUID data;
      procEvent.data().UnpackTo(&data);
      out << "uid"
          << "', '";
      out << data.process_pid() << "', '" << data.process_tgid() << "', '" << data.ruid() << "', '"
          << data.euid() << "', ";
      break;
    }
    case tkm::msg::server::ProcEvent::What::ProcEvent_What_GID: {
      tkm::msg::server::ProcEventGID data;
      procEvent.data().UnpackTo(&data);
      out << "gid"
          << "', '";
      out << data.process_pid() << "', '" << data.process_tgid() << "', '" << data.rgid() << "', '"
          << data.egid() << "', ";
      break;
    }
    default:
      break;
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
                    const tkm::msg::server::SysProcStat &sysProcStat,
                    uint64_t ts,
                    uint64_t recvTime) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_sysProcStatTableName << " ("
        << m_sysProcStatColumn.at(SysProcStatColumn::Timestamp) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::RecvTime) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatName) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatAll) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatUsr) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::CPUStatSys) << ","
        << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << ") VALUES ('" << ts << "', '"
        << recvTime << "', '" << sysProcStat.cpu().name() << "', '" << sysProcStat.cpu().all()
        << "', '" << sysProcStat.cpu().usr() << "', '" << sysProcStat.cpu().sys() << "', ";

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
                    const tkm::msg::server::SysProcMeminfo &sysProcMem,
                    uint64_t ts,
                    uint64_t recvTime) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_sysProcMeminfoTableName << " ("
        << m_sysProcMemColumn.at(SysProcMemColumn::Timestamp) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::RecvTime) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::MemTotal) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::MemFree) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::MemAvail) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::MemCached) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::MemAvailPercent) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SwapTotal) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SwapFree) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SwapCached) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SwapFreePercent) << ","
        << m_sysProcMemColumn.at(SysProcMemColumn::SessionId) << ") VALUES ('" << ts << "', '"
        << recvTime << "', '" << sysProcMem.mem_total() << "', '" << sysProcMem.mem_free() << "', '"
        << sysProcMem.mem_available() << "', '" << sysProcMem.mem_cached() << "', '"
        << sysProcMem.mem_percent() << "', '" << sysProcMem.swap_total() << "', '"
        << sysProcMem.swap_free() << "', '" << sysProcMem.swap_cached() << "', '"
        << sysProcMem.swap_percent() << "', ";

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
                    const tkm::msg::server::SysProcPressure &sysProcPressure,
                    uint64_t ts,
                    uint64_t recvTime) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_sysProcPressureTableName << " ("
        << m_sysProcPressureColumn.at(SysProcPressureColumn::Timestamp) << ","
        << m_sysProcPressureColumn.at(SysProcPressureColumn::RecvTime) << ","
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
        << m_sysProcStatColumn.at(SysProcStatColumn::SessionId) << ") VALUES ('" << ts << "', '"
        << recvTime << "', '" << sysProcPressure.cpu_some().avg10() << "', '"
        << sysProcPressure.cpu_some().avg60() << "', '" << sysProcPressure.cpu_some().avg300()
        << "', '" << sysProcPressure.cpu_some().total() << "', '"
        << sysProcPressure.cpu_full().avg10() << "', '" << sysProcPressure.cpu_full().avg60()
        << "', '" << sysProcPressure.cpu_full().avg300() << "', '"
        << sysProcPressure.cpu_full().total() << "', '" << sysProcPressure.mem_some().avg10()
        << "', '" << sysProcPressure.mem_some().avg60() << "', '"
        << sysProcPressure.mem_some().avg300() << "', '" << sysProcPressure.mem_some().total()
        << "', '" << sysProcPressure.mem_full().avg10() << "', '"
        << sysProcPressure.mem_full().avg60() << "', '" << sysProcPressure.mem_full().avg300()
        << "', '" << sysProcPressure.mem_full().total() << "', '"
        << sysProcPressure.io_some().avg10() << "', '" << sysProcPressure.io_some().avg60()
        << "', '" << sysProcPressure.io_some().avg300() << "', '"
        << sysProcPressure.io_some().total() << "', '" << sysProcPressure.io_full().avg10()
        << "', '" << sysProcPressure.io_full().avg60() << "', '"
        << sysProcPressure.io_full().avg300() << "', '" << sysProcPressure.io_full().total()
        << "', ";

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
                    const tkm::msg::server::ProcAcct &procAcct,
                    uint64_t ts,
                    uint64_t recvTime) -> std::string
{
  std::stringstream out;

  if ((type == Query::Type::SQLite3) || (type == Query::Type::PostgreSQL)) {
    out << "INSERT INTO " << m_procAcctTableName << " ("
        << m_procAcctColumn.at(ProcAcctColumn::Timestamp) << ","
        << m_procAcctColumn.at(ProcAcctColumn::RecvTime) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcComm) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcUid) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcGid) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcPid) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcPPid) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcUTime) << ","
        << m_procAcctColumn.at(ProcAcctColumn::AcSTime) << ","
        << m_procAcctColumn.at(ProcAcctColumn::CpuPercent) << ","
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
        << m_procAcctColumn.at(ProcAcctColumn::SessionId) << ") VALUES ('" << ts << "', '"
        << recvTime << "', '" << procAcct.ac_comm() << "', '" << procAcct.ac_uid() << "', '"
        << procAcct.ac_gid() << "', '" << procAcct.ac_pid() << "', '" << procAcct.ac_ppid()
        << "', '" << procAcct.ac_utime() << "', '" << procAcct.ac_stime() << "', '"
        << procAcct.cpu_percent() << "', '" << procAcct.cpu().cpu_count() << "', '"
        << procAcct.cpu().cpu_run_real_total() << "', '" << procAcct.cpu().cpu_run_virtual_total()
        << "', '" << procAcct.cpu().cpu_delay_total() << "', '"
        << procAcct.cpu().cpu_delay_average() << "', '" << procAcct.mem().coremem() << "', '"
        << procAcct.mem().virtmem() << "', '" << procAcct.mem().hiwater_rss() << "', '"
        << procAcct.mem().hiwater_vm() << "', '" << procAcct.ctx().nvcsw() << "', '"
        << procAcct.ctx().nivcsw() << "', '" << procAcct.swp().swapin_count() << "', '"
        << procAcct.swp().swapin_delay_total() << "', '" << procAcct.swp().swapin_delay_average()
        << "', '" << procAcct.io().blkio_count() << "', '" << procAcct.io().blkio_delay_total()
        << "', '" << procAcct.io().blkio_delay_average() << "', '" << procAcct.io().read_bytes()
        << "', '" << procAcct.io().write_bytes() << "', '" << procAcct.io().read_char() << "', '"
        << procAcct.io().write_char() << "', '" << procAcct.io().read_syscalls() << "', '"
        << procAcct.io().write_syscalls() << "', '" << procAcct.reclaim().freepages_count()
        << "', '" << procAcct.reclaim().freepages_delay_total() << "', '"
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

} // namespace tkm

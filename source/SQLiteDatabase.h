/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SQLiteDatabase Class
 * @details   SQLite3 database implementation
 *-
 */

#pragma once

#include "IDatabase.h"
#include "Options.h"

#include <any>
#include <sqlite3.h>

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::collector
{

class SQLiteDatabase : public IDatabase, public std::enable_shared_from_this<SQLiteDatabase>
{
public:
  enum class QueryType {
    Check,
    Create,
    DropTables,
    LoadDevices,
    GetDevices,
    GetSessions,
    AddDevice,
    RemDevice,
    HasDevice,
    AddSession,
    RemSession,
    HasSession,
    EndSession,
    CleanSessions,
    AddData,
  };

  typedef struct Query {
    QueryType type;
    void *raw;
  } Query;

public:
  explicit SQLiteDatabase(std::shared_ptr<Options> options);
  SQLiteDatabase(SQLiteDatabase const &) = delete;
  void operator=(SQLiteDatabase const &) = delete;

  void enableEvents() final;
  auto getShared() -> std::shared_ptr<SQLiteDatabase> { return shared_from_this(); }
  bool requestHandler(const IDatabase::Request &request) final;

  bool runQuery(const std::string &sql, Query &query);

public:
  SQLiteDatabase();
  ~SQLiteDatabase();

private:
  sqlite3 *m_db = nullptr;
};

} // namespace tkm::collector

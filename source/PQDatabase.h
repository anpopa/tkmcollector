/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     PQDatabase Class
 * @details   PostgreSQL database implementation
 *-
 */

#pragma once

#include "IDatabase.h"
#include "Options.h"

#include <any>
#include <pqxx/pqxx>

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::collector
{

class PQDatabase : public IDatabase, public std::enable_shared_from_this<PQDatabase>
{
public:
  PQDatabase(std::shared_ptr<Options> options);
  PQDatabase(PQDatabase const &) = delete;
  void operator=(PQDatabase const &) = delete;

  void enableEvents() final;
  auto getShared() -> std::shared_ptr<PQDatabase> { return shared_from_this(); }
  auto getConnection() -> std::unique_ptr<pqxx::connection> & { return m_connection; }
  bool requestHandler(const IDatabase::Request &request) final;

  auto runTransaction(const std::string &sql) -> pqxx::result;
  bool reconnect();

public:
  PQDatabase();
  ~PQDatabase() = default;

private:
  std::unique_ptr<pqxx::connection> m_connection = nullptr;
};

} // namespace tkm::collector

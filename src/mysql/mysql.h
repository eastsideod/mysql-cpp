// COPYRIGHT ...

#ifndef SRC_MYSQL_MYSQL_H_
#define SRC_MYSQL_MYSQL_H_

#include <functional>
#include <string>
#include <memory>

#include "./connection.h"

namespace mysql {

enum MysqlConnectionPoolCreateResult {
  kSuccess = 0,
  kDuplicated = 1,
  kError = 2,
};

class MysqlConnectionPool {
 public:
  using ConnectCallback = std::function<void(const size_t connectionPoolSize)>;
  using ExecuteQueryCallback = MysqlConnection::ExecuteQueryCallback;

  static MysqlConnectionPoolCreateResult CreateMysqlConnectionPool(
      const std::string &connection_pool_name,
      const std::string &id,
      const std::string &password,
      const std::string &database,
      const std::string &host,
      const uint16_t port,
      const size_t pool_size,
      MysqlConnectionPool *out);

  ~MysqlConnectionPool();

  void Connect(
      const std::string &connection_pool_name,
      const std::string &id,
      const std::string &password,
      const std::string &database,
      const std::string &host,
      const uint16_t port);

  void Stop();

  void ExecuteQuery(const std::string &query, const ExecuteQueryCallback &cb);

  const size_t GetExpectedPoolSize();
  const size_t GetActualPoolSize();

 private:
  MysqlConnectionPool(
      const std::string &connection_pool_name,
      const std::string &id,
      const std::string &password,
      const std::string &database,
      const std::string &host,
      const uint16_t port,
      const size_t pool_size);

  class MysqlConnectionPoolImpl;
  std::shared_ptr<MysqlConnectionPoolImpl> impl_;
};

}  // namespace mysql

#endif  // SRC_MYSQL_MYSQL_H_

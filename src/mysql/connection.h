// COPYRIGHT ...

#ifndef SRC_MYSQL_CONNECTION_H_
#define SRC_MYSQL_CONNECTION_H_

#include <functional>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <memory>
#include <string>

namespace mysql {

using ResultSet = sql::ResultSet;
using SQLException = sql::SQLException;

class MysqlConnection {
 public:
  using ExecuteQueryCallback =
      std::function<
          void(const std::shared_ptr<SQLException> &/*error*/,
               const std::shared_ptr<ResultSet> &/*result*/)>;

  using ConnectCallback =
      std::function<void(const std::shared_ptr<SQLException> &/*error*/)>;

  using StopCallback =
      std::function<void(const bool /*success*/, const bool /*alreadyClosed*/)>;

  explicit MysqlConnection(sql::Driver *driver)
      : driver_(driver), conn_(nullptr) {
  }

  ~MysqlConnection();

  // FIXME(inkeun): query timeout 관련 기본 옵션 추가
  void Connect(
      const std::string &connection_pool_name,
      const std::string &id,
      const std::string &password,
      const std::string &database,
      const std::string &host,
      const uint16_t port,
      const ConnectCallback &cb);

  void ExecuteQuery(const std::string &query, const ExecuteQueryCallback &cb);
  void Stop(const StopCallback &cb);

 private:
  sql::Driver *driver_;
  std::shared_ptr<sql::Connection> conn_;
};

}  // namespace mysql

#endif  // SRC_MYSQL_CONNECTION_H_

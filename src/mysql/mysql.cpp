// COPYRIGHT ...

#include "mysql.h"

#include <cppconn/driver.h>

#include <chrono>
#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>

#include <boost/asio.hpp>

#include "./connection.h"
#include "../io_service/io_service.h"


namespace mysql {

namespace {

using ConnectCallback = MysqlConnectionPool::ConnectCallback;
using ExecuteQueryCallback = MysqlConnectionPool::ExecuteQueryCallback;

struct QueryContext {
  QueryContext(
    const std::string &query,
    const ExecuteQueryCallback &cb,
    const std::chrono::time_point<std::chrono::system_clock> &queryStartedAt)
    : query_(query),
      cb_(cb),
      queryStartedAt_(queryStartedAt) {
  }

  const std::string query_;
  const ExecuteQueryCallback cb_;
  const std::chrono::time_point<std::chrono::system_clock> queryStartedAt_;
};


std::unordered_map<std::string /*connectionPoolName*/, MysqlConnectionPool>
    ThePool;


}  // unnamed namespace


////////////////////////////////////////////////////////////////////////////////
// MysqlConnectionPoolImpl

class MysqlConnectionPool::MysqlConnectionPoolImpl {
 public:
  MysqlConnectionPoolImpl(
      const std::string &connection_pool_name,
      const std::string &id,
      const std::string &password,
      const std::string &database,
      const std::string &host,
      const uint16_t port,
      const size_t pool_size)
      : connection_pool_name_(connection_pool_name),
        id_(id),
        password_(password),
        database_(database),
        host_(host),
        port_(port),
        pool_size_(pool_size),
        io_service_(std::make_shared<IoService>(pool_size)),
        driver_(get_driver_instance()) {
  }

  ~MysqlConnectionPoolImpl() {
    driver_ = nullptr;
  }

  void Connect(
      const std::string &connection_pool_name,
      const std::string &id,
      const std::string &password,
      const std::string &database,
      const std::string &host,
      const uint16_t port) {
    for (int i = 0; i < pool_size_; ++i) {
      const auto conn = std::make_shared<MysqlConnection>(driver_);

      conn->Connect(
        connection_pool_name,
        id, password, database,
        host, port,
        std::bind(&MysqlConnectionPoolImpl::PostProcessConnectionConnected,
                  this, conn, std::placeholders::_1));
    }
  }

  void Stop() {
    // FIXME(inkeun): flush all waiting query contexts;

    std::queue<std::shared_ptr<MysqlConnection>> queue;
    available_mysql_connectors_.swap(queue);

    for (const auto &conn : total_mysql_connectors_) {
      io_service_->Post([this, conn](){
        conn->Stop(
          std::bind(&MysqlConnectionPoolImpl::PostProcessStopped,
                    this, conn, std::placeholders::_1, std::placeholders::_2));
      });
    }
  }

  void ExecuteQuery(const std::string &query, const ExecuteQueryCallback &cb) {
    if (available_mysql_connectors_.empty()) {
      QueryContext qc(query, cb, std::chrono::system_clock::now());
      query_contexts.emplace(qc);
      return;
    }

    const auto &conn = available_mysql_connectors_.front();
    available_mysql_connectors_.pop();

    io_service_->Post([this, conn, query, cb](){
      conn->ExecuteQuery(
          query,
          std::bind(&MysqlConnectionPoolImpl::PostProcessQueryExecuted,
                    this, conn,
                    std::placeholders::_1,
                    std::placeholders::_2, cb));
    });
  }

  const size_t GetExpectedPoolSize() {
    return pool_size_;
  }

  const size_t GetActualPoolSize() {
    return actual_pool_size_;
  }

 private:
  void PostProcessConnectionConnected(
      const std::shared_ptr<MysqlConnection> &conn,
      const std::shared_ptr<SQLException> &error) {
    if (error) {
      // TODO(inkeun): reset conn.
      return;
    }

    ++actual_pool_size_;
    available_mysql_connectors_.emplace(conn);
  }

  void PostProcessStopped(const std::shared_ptr<MysqlConnection> &conn,
                          const bool success,
                          const bool alreadyClosed) {
    if (success && !alreadyClosed) {
      --actual_pool_size_;
    }

    if (actual_pool_size_ == 0) {
      total_mysql_connectors_.clear();
    }
  }

  void PostProcessQueryExecuted(
      const std::shared_ptr<MysqlConnection> &conn,
      const std::shared_ptr<SQLException> &error,
      const std::shared_ptr<ResultSet> &result,
      const ExecuteQueryCallback &cb) {
    available_mysql_connectors_.emplace(conn);
    cb(error, result);
  }

  const std::string connection_pool_name_;
  const std::string id_;
  const std::string password_;
  const std::string database_;
  const std::string host_;
  const uint16_t port_;
  const size_t pool_size_;  // expected

  size_t actual_pool_size_;
  std::shared_ptr<IoService> io_service_;

  sql::Driver *driver_;
  std::queue<QueryContext> query_contexts;
  std::vector<std::shared_ptr<MysqlConnection>> total_mysql_connectors_;
  std::queue<std::shared_ptr<MysqlConnection>> available_mysql_connectors_;
};


////////////////////////////////////////////////////////////////////////////////
// MysqlConnectionPool

MysqlConnectionPoolCreateResult MysqlConnectionPool::CreateMysqlConnectionPool(
    const std::string &connection_pool_name,
    const std::string &id,
    const std::string &password,
    const std::string &database,
    const std::string &host,
    const uint16_t port,
    const size_t pool_size,
    MysqlConnectionPool *out) {
  return MysqlConnectionPoolCreateResult::kSuccess;
}

MysqlConnectionPool::~MysqlConnectionPool() = default;

MysqlConnectionPool::MysqlConnectionPool(
    const std::string &connection_pool_name,
    const std::string &id,
    const std::string &password,
    const std::string &database,
    const std::string &host,
    const uint16_t port,
    const size_t pool_size)
    : impl_(std::make_shared<MysqlConnectionPoolImpl>(
                connection_pool_name, id, password,
                database, host, port, pool_size)) {
}

void MysqlConnectionPool::Connect(
    const std::string &connection_pool_name,
    const std::string &id,
    const std::string &password,
    const std::string &database,
    const std::string &host,
    const uint16_t port
) {
  impl_->Connect(connection_pool_name, id, password, database, host, port);
}


}  // namespace mysql

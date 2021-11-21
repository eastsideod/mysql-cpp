// COPYRIGHT ...

#include "connection.h"

namespace mysql {

namespace {

// raii guard sql::statement auto release
class StatementGuard {
 public:
  explicit StatementGuard(const sql::Statement *statement)
      :statement_(statement) {
  }

  ~StatementGuard() {
    delete statement_;
  }

 private:
  const sql::Statement *statement_;
};

}  // unnamed namespace


MysqlConnection::~MysqlConnection() {
  if (conn_) {
    conn_.reset();
  }
}


void MysqlConnection::Connect(
    const std::string &connection_pool_name,
    const std::string &id,
    const std::string &password,
    const std::string &database,
    const std::string &host,
    const uint16_t port,
    const ConnectCallback &cb) {
  try {
    sql::Connection *rawConn = driver_->connect(host, id, password);

    conn_ = std::shared_ptr<sql::Connection>(rawConn);
    conn_->setSchema(database);
    cb(nullptr);
  } catch (sql::SQLException &e) {
    cb(std::make_shared<sql::SQLException>(e));
  }
}


void MysqlConnection::ExecuteQuery(
    const std::string &query,
    const ExecuteQueryCallback &cb) {
  if (!conn_) {
    return;
  }

  // if connection lost....
  if (!conn_->isClosed() || !conn_->isValid()) {
    conn_->reconnect();
  }

  try {
    sql::Statement *statement = conn_->createStatement();
    StatementGuard guard(statement);

    sql::ResultSet *rawRs = statement->executeQuery(query);

    const std::shared_ptr<sql::ResultSet> rs =
        std::shared_ptr<sql::ResultSet>(rawRs);

    cb(nullptr, rs);
  } catch(sql::SQLException &e) {
    cb(std::make_shared<sql::SQLException>(e), nullptr);
  }
}


void MysqlConnection::Stop(const StopCallback &cb) {
  if (this->conn_->isClosed()) {
    return cb(true, true);
  }

  try {
    this->conn_->close();
    return cb(true, false);
  } catch(sql::SQLException &e) {
    // TODO(inkeun): 이곳에 에러 로그 추가
    return cb(false, false);
  }
}

}  // namespace mysql

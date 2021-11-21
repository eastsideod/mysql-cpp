// COPYRIGHT ...

#ifndef SRC_IO_SERVICE_IO_SERVICE_H_
#define SRC_IO_SERVICE_IO_SERVICE_H_

#include <functional>
#include <memory>

namespace mysql {

class IoService {
 public:
  explicit IoService(const size_t size);
  ~IoService();

  void Post(const std::function<void()> &func);
  void Stop();
 private:
  class IoServiceImpl;
  std::unique_ptr<IoServiceImpl> impl_;
};

}  // namespace mysql

#endif  // SRC_IO_SERVICE_IO_SERVICE_H_

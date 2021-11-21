#include "io_service.h"

#include <memory>
#include <thread>
#include <vector>
#include <boost/asio.hpp>


namespace mysql {

////////////////////////////////////////////////////////////////////////////////
// IoServiceImpl

class IoService::IoServiceImpl {
 public:
  explicit IoServiceImpl(const size_t size)
    : size_(size) {

    work_ = std::make_shared<boost::asio::io_service::work>(io_service_);

    for (size_t i = 0; i < size_; ++i) {
      threads_.push_back(
          std::thread(
              std::bind(&IoService::IoServiceImpl::Run, this)));
    }
  }

  void Run() {
    io_service_.run();
  }

  void Post(const std::function<void()> &func) {
    io_service_.post(func);
  }

  void Stop() {
    io_service_.stop();

    while (!io_service_.stopped()) {
      // looping wait for io_service stop.
    }

    work_.reset();

    for (auto &th : threads_) {
      if (th.joinable()) {
        th.join();
      }
    }
  }

 private:
  const size_t size_;
  std::shared_ptr<boost::asio::io_service::work> work_;
  boost::asio::io_service io_service_;
  std::vector<std::thread> threads_;
};


////////////////////////////////////////////////////////////////////////////////
// IoService

IoService::IoService(const size_t size) {
  impl_ = std::make_unique<IoServiceImpl>(size);
}

IoService::~IoService() {
  impl_->Stop();
}

void IoService::Post(const std::function<void()> &func) {
  impl_->Post(func);
}

void IoService::Stop() {
  impl_->Stop();
}

}  // namespace mysql

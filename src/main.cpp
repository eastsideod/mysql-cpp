#include "mysql/mysql.h"
#include "io_service/io_service.h"

#include <iostream>
#include <chrono>
#include <thread>

int main() {
  mysql::MysqlConnectionPool::CreateMysqlConnectionPool(
    "1", "2", "3", "4", "5", 3306, 5, NULL);

  mysql::IoService io_service(5);
  std::this_thread::sleep_for(std::chrono::seconds(2));

  for (int i = 0; i < 10; ++i) {
    io_service.Post([i](){
      std::cout<< "hi!!! i="<< i <<  ""<< std::this_thread::get_id() <<  std::endl;

    });
  }

  // try {
    // while(true) {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      io_service.Stop();

  // } catch(std::exception e) {
  //   std::cout << e.what() << std::endl;
  // }


  std::cout << "hihi" << std::endl;
  return 0;
}
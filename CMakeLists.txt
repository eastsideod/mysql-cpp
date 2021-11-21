cmake_minimum_required (VERSION 3.1.0)
project(mysqlcpp C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

find_package(Boost REQUIRED system)

include_directories(/usr/include/cppconn)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(mysqlcpp ${Boost_LIBRARY_DIRS})

set(HEADER
  ${CMAKE_SOURCE_DIR}/src/mysql/mysql.h
  ${CMAKE_SOURCE_DIR}/src/mysql/connection.h
  ${CMAKE_SOURCE_DIR}/src/io_service/io_service.h
)

set(CPP
  ${CMAKE_SOURCE_DIR}/src/mysql/mysql.cpp
  ${CMAKE_SOURCE_DIR}/src/mysql/connection.cpp
  ${CMAKE_SOURCE_DIR}/src/io_service/io_service.cpp
)

set(SOURCES ${HEADER} ${CPP})

add_executable(mysqlcpp
  ${HEADER}
  ${SOURCES}
  ${CMAKE_SOURCE_DIR}/src/main.cpp)

target_link_libraries(mysqlcpp PUBLIC
  ${Boost_LIBRARIES}
  mysqlcppconn
  pthread)


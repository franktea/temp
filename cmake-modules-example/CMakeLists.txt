cmake_minimum_required(VERSION 3.28)
project(cmake_module_example CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(foo)

target_sources(foo
  PUBLIC
    FILE_SET CXX_MODULES FILES
      hello.cpp
)

add_executable(hello main.cpp)
target_link_libraries(hello foo)
cmake_minimum_required(VERSION 3.28)
project(enc_dec)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

add_library(enc_dec_wrapper MODULE enc_dec_wrapper.cpp)
target_include_directories(enc_dec_wrapper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11)
include_directories(${Python3_INCLUDE_DIRS} ${pybind11_INCLUDE_DIRS})
target_link_libraries(enc_dec_wrapper ${Python3_LIBRARIES})

#install(TARGETS enc_dec_wrapper LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(main main.cpp)
target_link_libraries(main ${Python3_LIBRARIES})

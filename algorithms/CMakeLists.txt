cmake_minimum_required(VERSION 2.8)
project(algorithm_test)

add_executable(algorithmtest test_main.cpp bst_test.cpp)

target_link_libraries(algorithmtest gtest pthread)
cmake_minimum_required(VERSION 3.20)

project(mac_cursor_center)

add_executable(center_cursor mac_cursor_center.cpp)

set_property(TARGET center_cursor PROPERTY CXX_STANDARD 20)

target_link_libraries(center_cursor PRIVATE "-framework ApplicationServices")

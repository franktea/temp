cmake_minimum_required(VERSION 3.28)
project(enc_dec)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

find_program(Python3_CONFIG_EXECUTABLE python3-config)
if(NOT Python3_CONFIG_EXECUTABLE)
    message(FATAL_ERROR "python3-config not found")
endif()

# 将默认生成的libenc_dec_wrapper.so命名为enc_dec_wrapper.cpython-312-darwin.so
# 注意只有macos下才是这样的逻辑，其它系统自己去改
execute_process(COMMAND ${Python3_CONFIG_EXECUTABLE} --extension-suffix
                OUTPUT_VARIABLE Python3_EXTENSION_SUFFIX
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(${Python3_EXTENSION_SUFFIX})

add_library(enc_dec_wrapper MODULE enc_dec_wrapper.cpp)
set_target_properties(enc_dec_wrapper PROPERTIES PREFIX "")
set_target_properties(enc_dec_wrapper PROPERTIES SUFFIX "${Python3_EXTENSION_SUFFIX}")
target_include_directories(enc_dec_wrapper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11)
include_directories(${Python3_INCLUDE_DIRS} ${pybind11_INCLUDE_DIRS})
target_link_libraries(enc_dec_wrapper ${Python3_LIBRARIES})


add_executable(main main.cpp)
target_link_libraries(main ${Python3_LIBRARIES})

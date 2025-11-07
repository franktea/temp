cmake_minimum_required(VERSION 3.16)

project(http_stdexec VERSION 0.1 LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
  stdexec
  GIT_REPOSITORY https://github.com/NVIDIA/stdexec
  GIT_TAG main
)

FetchContent_MakeAvailable(stdexec)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)

qt_add_executable(http_stdexec
    qtstdexec.h
    main.cpp
    httpdownloader.h 
    httpdownloader.cpp
)

qt_add_qml_module(http_stdexec
    URI http_example
    VERSION 1.0
    QML_FILES main.qml 
)

target_compile_definitions(http_stdexec
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_include_directories(http_stdexec
    PRIVATE ${stdexec_SOURCE_DIR}/include)

target_link_libraries(http_stdexec
    PRIVATE Qt6::Quick
            stdexec)

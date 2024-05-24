cmake_minimum_required(VERSION 3.28)

project(senders-qt LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

CPMAddPackage(NAME stdexec
    GITHUB_REPOSITORY NVIDIA/stdexec
    GIT_TAG main
)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(senders-qt
  main.cpp
  qtstdexec.h
)
target_link_libraries(senders-qt Qt${QT_VERSION_MAJOR}::Core STDEXEC::stdexec)

include(GNUInstallDirs)
install(TARGETS senders-qt
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

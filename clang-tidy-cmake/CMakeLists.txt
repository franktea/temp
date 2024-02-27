cmake_minimum_required(VERSION 3.6)
project(clang-tidy-example)

set(CMAKE_CXX_STANDARD 20)

# generate hello world file
file(
  GENERATE
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/hello.cpp
  CONTENT
    "#include <iostream>\n int main() { std::cout << \"Hello World\" << std::endl; exit(0); }"
)

# search for clang-tidy
find_program(CLANG_TIDY_EXE NAMES "clang-tidy" REQUIRED)

# setup clang-tidy command from executable + options
set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}" "-checks=-*,modernize-*")

# add target using generated source file
add_executable(hello ${CMAKE_CURRENT_BINARY_DIR}/hello.cpp)

# set CXX_CLANG_TIDY property after defining the target
set_target_properties(hello PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")

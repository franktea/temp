cmake_minimum_required(VERSION 3.10)
project(ConfigDemo LANGUAGES CXX)

# Read the config.txt file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/config.txt" CONFIG_TXT_CONTENT)

# Escape the content for C string: replace newlines with \n and escape quotes and backslashes
string(REPLACE "\\" "\\\\" ESCAPED_CONTENT "${CONFIG_TXT_CONTENT}")
string(REPLACE "\"" "\\\"" ESCAPED_CONTENT "${ESCAPED_CONTENT}")
string(REPLACE "\n" "\\n\"\n\"" ESCAPED_CONTENT "${ESCAPED_CONTENT}")

# Remove the trailing "\n\"" from the last line (if any)
# We'll handle by appending a terminating newline appropriately.
# Instead, we can do a simpler approach: replace newline with \n and keep as a single line.
# Let's do: replace newline with \n and keep as one line.
string(REPLACE "\n" "\\n" ESCAPED_CONTENT_ONE_LINE "${CONFIG_TXT_CONTENT}")
string(REPLACE "\"" "\\\"" ESCAPED_CONTENT_ONE_LINE "${ESCAPED_CONTENT_ONE_LINE}")

# Now configure the header file
configure_file(config.h.in config.h @ONLY)

# Add executable
add_executable(config_demo main.cpp)

# Include the current binary directory for the generated header
target_include_directories(config_demo PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
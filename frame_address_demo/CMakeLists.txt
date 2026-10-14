cmake_minimum_required(VERSION 3.16)
project(frame_address_demo CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# 主演示：-O0 且显式保留帧指针，栈帧布局可预期、帧指针链完整，
# 方便观察局部变量的顺序、对齐填充，以及沿帧指针链回溯调用栈。
# ---------------------------------------------------------------------------
add_executable(frame_address_demo src/main.cpp)
target_compile_options(frame_address_demo PRIVATE
    -O0
    -fno-omit-frame-pointer
    -Wall -Wextra
)

# ---------------------------------------------------------------------------
# 对比演示：-O2 默认会省略帧指针（-fomit-frame-pointer），
# 此时 __builtin_frame_address(1)/__builtin_return_address(1) 及更高层级
# 会变得不可靠。为避免解引用野指针导致崩溃，该目标以 SAFE_ONLY 模式
# 只打印地址值、不做帧指针链遍历。
# ---------------------------------------------------------------------------
add_executable(frame_address_demo_omitfp src/main.cpp)
target_compile_options(frame_address_demo_omitfp PRIVATE
    -O2
    -DDEMO_SAFE_ONLY
    -Wall -Wextra
)

# dladdr 在 Linux 上位于 libdl，在 macOS 上位于 libSystem（无需额外链接）
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(frame_address_demo PRIVATE dl)
    target_link_libraries(frame_address_demo_omitfp PRIVATE dl)
endif()

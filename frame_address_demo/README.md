# frame_address_demo

演示 GCC/Clang 内建函数 `__builtin_frame_address` 与 `__builtin_return_address`
的最小可运行工程，基于 CMake 构建。

## 构建与运行

```bash
cmake -S . -B build
cmake --build build
./build/frame_address_demo
./build/frame_address_demo_omitfp
```

## 两个目标

| 目标 | 编译选项 | 说明 |
|------|----------|------|
| `frame_address_demo` | `-O0 -fno-omit-frame-pointer` | 帧指针链完整，可稳定回溯调用栈、观察局部变量布局 |
| `frame_address_demo_omitfp` | `-O2`（默认省略帧指针） | 演示高层级 `frame_address`/`return_address` 失效 |

## 依赖

- 任意较新的 CMake（>= 3.16）
- 支持内建函数的编译器：GCC 或 Clang（含 Apple Clang）
- Linux 下 `dladdr` 需要链接 `-ldl`（已在 CMakeLists.txt 中处理）；
  macOS 下位于 libSystem，无需额外链接。

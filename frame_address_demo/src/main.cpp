// frame_address_demo —— 演示 GCC/Clang 内建函数 __builtin_frame_address
// 以及 __builtin_return_address 的用法。
//
// 本工程包含两个构建目标：
//   * frame_address_demo        —— -O0 -fno-omit-frame-pointer，帧指针链完整
//   * frame_address_demo_omitfp —— -O2（默认省略帧指针），演示高层级地址失效
//
// 在 arm64（Apple Silicon / Linux）与 x86-64 上均可用：
//   两者的“帧记录”布局一致： [fp] = 上层 fp，[fp+8] = 返回地址。

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>

#if defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#endif

namespace {

// 把某个地址解析成它所属的符号名（利用 dladdr）。
// macOS 上位于 libSystem，无需额外链接；Linux 上需要链接 -ldl。
std::string symbol_name(void* addr)
{
#if defined(__APPLE__) || defined(__linux__)
    Dl_info info{};
    if (addr != nullptr && dladdr(addr, &info) != 0 && info.dli_sname != nullptr) {
        return std::string(info.dli_sname);
    }
#else
    (void)addr;
#endif
    return "??";
}

// ---------------------------------------------------------------------------
// 1. __builtin_frame_address(0)：探秘“当前”函数的栈帧
// ---------------------------------------------------------------------------
[[gnu::noinline]] void explore_current_frame()
{
    std::cout << "\n============ 1. __builtin_frame_address(0)：探秘当前栈帧 ============\n";

    // 返回当前函数帧指针的值（arm64 上是 x29，x86-64 上是 rbp）。
    void* fp = __builtin_frame_address(0);

    // 故意声明不同类型、不同对齐要求的局部变量，观察它们在栈上的相对位置。
    int   a = 0x11111111;
    short b = 0x2222;
    char  c = 0x33;
    long  d = 0x4444444444444444L;
    int   e = 0x55555555;

    auto addr = [](const void* p) { return reinterpret_cast<std::uintptr_t>(p); };

    std::cout << "frame pointer (fp) = " << fp << '\n';
    std::cout << "&a (int ) = " << static_cast<void*>(&a) << '\n';
    std::cout << "&b (short) = " << static_cast<void*>(&b) << '\n';
    std::cout << "&c (char ) = " << static_cast<void*>(&c) << '\n';
    std::cout << "&d (long ) = " << static_cast<void*>(&d) << '\n';
    std::cout << "&e (int ) = " << static_cast<void*>(&e) << '\n';

    // 近似估算当前栈帧占用：帧指针到“最低”局部变量的距离。
    std::uintptr_t lowest = addr(&a);
    lowest = std::min(lowest, addr(&b));
    lowest = std::min(lowest, addr(&c));
    lowest = std::min(lowest, addr(&d));
    lowest = std::min(lowest, addr(&e));

    std::cout << "approx. frame usage ~ "
              << (addr(fp) > lowest ? addr(fp) - lowest : 0) << " bytes\n";

    (void)a; (void)b; (void)c; (void)d; (void)e;
}

// ---------------------------------------------------------------------------
// 2. level 参数：0 是当前函数，1/2/3 逐层向上回溯
// ---------------------------------------------------------------------------
[[gnu::noinline]] void level_demo()
{
    std::cout << "\n============= 2. level 参数：frame_address / return_address =============\n";

    // 注意：level 必须是编译期常量，不能写成循环变量 i。
    void* fa0 = __builtin_frame_address(0);
    void* fa1 = __builtin_frame_address(1);
    void* fa2 = __builtin_frame_address(2);

    void* ra0 = __builtin_return_address(0);
    void* ra1 = __builtin_return_address(1);
    void* ra2 = __builtin_return_address(2);

    std::cout << "level 0: frame_address = " << fa0
              << "   return_address = " << ra0 << "  (" << symbol_name(ra0) << ")\n";
    std::cout << "level 1: frame_address = " << fa1
              << "   return_address = " << ra1 << "  (" << symbol_name(ra1) << ")\n";
    std::cout << "level 2: frame_address = " << fa2
              << "   return_address = " << ra2 << "  (" << symbol_name(ra2) << ")\n";
}

// ---------------------------------------------------------------------------
// 3. 沿“帧指针链”手搓一个轻量 backtrace
//    arm64 与 x86-64 的帧记录布局一致：[fp] = 上层 fp，[fp+8] = 返回地址
// ---------------------------------------------------------------------------
[[gnu::noinline]] void walk_frames()
{
#if defined(DEMO_SAFE_ONLY)
    // 省略帧指针时，解引用上层 fp 可能读到野指针导致崩溃，因此只打印地址值。
    std::cout << "\n=== 3. (SAFE_ONLY) 省略帧指针后，高层级地址失效 ===\n";

    void* fa0 = __builtin_frame_address(0);
    void* fa1 = __builtin_frame_address(1);
    void* fa2 = __builtin_frame_address(2);

    void* ra0 = __builtin_return_address(0);
    void* ra1 = __builtin_return_address(1);
    void* ra2 = __builtin_return_address(2);

    std::cout << "frame_address(0) = " << fa0 << '\n';
    std::cout << "frame_address(1) = " << fa1 << "  <-- 已不可靠\n";
    std::cout << "frame_address(2) = " << fa2 << "  <-- 已不可靠\n";
    std::cout << "return_address(0) = " << ra0 << "  (" << symbol_name(ra0) << ")\n";
    std::cout << "return_address(1) = " << ra1 << "  (" << symbol_name(ra1)
              << ")  <-- 已不可靠\n";
    std::cout << "return_address(2) = " << ra2 << "  (" << symbol_name(ra2)
              << ")  <-- 已不可靠\n";
#else
    std::cout << "\n================ 3. 沿帧指针链回溯调用栈 ================\n";

    std::uintptr_t fp = reinterpret_cast<std::uintptr_t>(__builtin_frame_address(0));
    const int kMaxFrames = 32;
    int depth = 0;

    while (fp != 0 && depth < kMaxFrames) {
        // 帧记录：[fp] = 上层帧指针，[fp+8] = 本帧的返回地址
        std::uintptr_t caller_fp = *reinterpret_cast<std::uintptr_t*>(fp);
        std::uintptr_t ret_addr  = *reinterpret_cast<std::uintptr_t*>(fp + 8);

        void* ret = reinterpret_cast<void*>(ret_addr);
        std::cout << "frame " << std::setw(2) << depth
                  << "  fp = " << reinterpret_cast<void*>(fp)
                  << "  ret = " << ret
                  << "  (" << symbol_name(ret) << ")\n";

        fp = caller_fp;
        ++depth;
    }
#endif
}

// 构造一个 4 层调用链：main -> top -> middle -> deepest
[[gnu::noinline]] void deepest()
{
    explore_current_frame();
    level_demo();
    walk_frames();
}

[[gnu::noinline]] void middle()
{
    deepest();
}

[[gnu::noinline]] void top()
{
    middle();
}

}  // namespace

int main()
{
    std::cout << "===== __builtin_frame_address 演示 =====\n";
#if defined(__x86_64__)
    std::cout << "arch: x86-64\n";
#elif defined(__aarch64__)
    std::cout << "arch: arm64\n";
#else
    std::cout << "arch: unknown\n";
#endif

    top();
    return 0;
}

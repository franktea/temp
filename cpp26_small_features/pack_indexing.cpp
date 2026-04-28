// C++26 Pack Indexing - P2662R3
// 演示包索引（pack indexing）功能
// 编译: g++ -std=c++26 -o pack_indexing pack_indexing.cpp && ./pack_indexing

#include <iostream>
#include <string>

// 使用 auto 返回类型推导，避免在返回类型中使用 T...[N]
// （GCC 16 对返回类型中的类型包索引支持有限）

// 获取参数包中第 N 个元素的值
template <typename... T>
constexpr auto first(T... values) -> decltype(auto)
{
    return values...[0];    // values...[0] 取第一个值
}

// 获取参数包中最后一个元素
template <typename... T>
constexpr auto last(T... values) -> decltype(auto)
{
    return values...[sizeof...(values) - 1];
}

// 获取参数包中索引为 I 的元素
template <size_t I, typename... T>
constexpr auto get(T... values) -> decltype(auto)
{
    return values...[I];
}

// 使用编译期表达式作为索引
template <typename... T>
    requires (sizeof...(T) >= 2)
constexpr auto sum_first_and_last(T... values) -> decltype(auto)
{
    return values...[0] + values...[sizeof...(values) - 1];
}

// 在泛型 lambda 中使用包索引
void demo_lambda_pack_index()
{
    auto print_third = []<typename... T>(T... args) {
        std::cout << "第三个参数是: " << args...[2] << '\n';
    };
    print_third(10, 20, 30, 40, 50);
}

// 别名模板中使用类型包索引
template <typename... T>
using first_type = T...[0];

template <typename... T>
using last_type = T...[sizeof...(T) - 1];

int main()
{
    std::cout << "=== C++26 Pack Indexing 演示 ===\n\n";

    // 基本用法
    std::cout << "first(1, 2, 3, 4, 5) = " << first(1, 2, 3, 4, 5) << '\n';
    std::cout << "last(1, 2, 3, 4, 5)  = " << last(1, 2, 3, 4, 5) << '\n';

    // 任意索引
    std::cout << "get<2>(10, 20, 30, 40) = " << get<2>(10, 20, 30, 40) << '\n';

    // 字符串
    std::cout << "get<0>(\"hello\", \"world\") = " << get<0>("hello", "world") << '\n';

    // 编译期表达式索引
    std::cout << "sum_first_and_last(1, 2, 3, 4, 5) = "
              << sum_first_and_last(1, 2, 3, 4, 5) << '\n';

    // 混合类型
    std::cout << "first(42, 3.14, \"hello\") = " << first(42, 3.14, "hello") << '\n';

    // 验证类型别名
    static_assert(__is_same(first_type<int, double, char>, int));
    static_assert(__is_same(last_type<int, double, char>, char));

    // lambda 演示
    demo_lambda_pack_index();

    // 静态断言验证编译期能力
    static_assert(first(1, 2, 3, 4, 5) == 1);
    static_assert(last(1, 2, 3, 4, 5) == 5);
    static_assert(get<3>(10, 20, 30, 40) == 40);

    std::cout << "\n所有静态断言通过！\n";
    return 0;
}

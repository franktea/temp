// C++26 Structured Bindings can introduce a Pack - P1061R10
// 演示结构化绑定引入参数包
// 注意: GCC 16 支持在模板上下文中使用结构化绑定包
// 编译: g++ -std=c++26 -o sb_pack structured_bindings_pack.cpp && ./sb_pack

#include <iostream>
#include <tuple>
#include <string>
#include <cassert>

// ============ 辅助工具 ============

// 打印任意数量的参数
void print_all()
{
    std::cout << '\n';
}

template <typename T, typename... Rest>
void print_all(const T& first, const Rest&... rest)
{
    std::cout << first << ' ';
    print_all(rest...);
}

// ============ 基本用法 (模板上下文) ============

// 示例1: 将元组全部解包为参数包
template <typename Tuple>
void demo_full_pack(Tuple&& t)
{
    std::cout << "--- 全部绑定为包 ---\n";
    // C++26: [...xs] 将整个元组分解为一个包
    auto&& [...xs] = std::forward<Tuple>(t);
    print_all(xs...);
}

// 示例2: head + tail 分解
template <typename Tuple>
void demo_head_tail(Tuple&& t)
{
    std::cout << "\n--- 头部 + 尾部包 ---\n";
    // 第一个元素绑定到 a，其余作为包 rest
    auto [a, ...rest] = std::forward<Tuple>(t);

    std::cout << "第一个元素: " << a << '\n';
    std::cout << "剩余元素: ";
    print_all(rest...);
}

// 示例3: 头 + 中间包 + 尾
template <typename Tuple>
void demo_head_middle_tail(Tuple&& t)
{
    std::cout << "\n--- 头部 + 中间包 + 尾部 ---\n";
    // 第一个绑定到 a，最后一个绑定到 b，中间作为包 mid
    auto [a, ...mid, b] = std::forward<Tuple>(t);

    std::cout << "第一个: " << a << '\n';
    std::cout << "最后一个: " << b << '\n';
    std::cout << "中间元素: ";
    print_all(mid...);
}

// 示例4: 忽略元素
template <typename Tuple>
void demo_ignore(Tuple&& t)
{
    std::cout << "\n--- 忽略特定元素 ---\n";
    // C++26: 使用 _ 忽略某些元素
    auto [first, _, ...rest] = std::forward<Tuple>(t);

    std::cout << "第一个: " << first << '\n';
    std::cout << "剩余(跳过第二个): ";
    print_all(rest...);
}

// ============ 实际应用: 点积计算 ============

// C++26 之前: 需要嵌套 std::apply
template <class P, class Q>
auto dot_product_before_cpp26(P p, Q q)
{
    return std::apply([&](auto... p_elems) {
        return std::apply([&](auto... q_elems) {
            return (... + (p_elems * q_elems));
        }, q);
    }, p);
}

// C++26: 使用结构化绑定包，无需嵌套
template <class P, class Q>
auto dot_product_cpp26(P p, Q q)
{
    // 直接将元组分解为包，无需任何间接层！
    auto&& [...p_elems] = p;
    auto&& [...q_elems] = q;
    return (... + (p_elems * q_elems));
}

void demo_dot_product()
{
    std::cout << "\n--- 点积计算 (实际应用) ---\n";
    auto a = std::tuple{1, 2, 3};
    auto b = std::tuple{4, 5, 6};

    auto result_before = dot_product_before_cpp26(a, b);
    auto result_cpp26  = dot_product_cpp26(a, b);

    // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    std::cout << "点积 (C++17 嵌套 apply): " << result_before << '\n';
    std::cout << "点积 (C++26 包绑定):    " << result_cpp26 << '\n';
    assert(result_before == result_cpp26);
    std::cout << "结果一致，验证通过！\n";
}

// ============ 在模板中转换元组 ============

// 将元组转换为字符串（模板版本）
template <typename... T>
std::string tuple_to_string(const std::tuple<T...>& t)
{
    auto&& [...elems] = t;
    std::string result;
    auto append = [&result](const auto& elem) {
        using std::to_string;
        if constexpr (std::is_same_v<std::decay_t<decltype(elem)>,
                                     std::string>) {
            result += elem + ", ";
        } else {
            result += to_string(elem) + ", ";
        }
    };
    (append(elems), ...);
    if (!result.empty()) result.resize(result.size() - 2);
    return "[" + result + "]";
}

void demo_tuple_to_string()
{
    std::cout << "\n--- 元组转字符串 ---\n";
    auto t = std::tuple{42, 3.14, std::string{"hello"}};
    std::cout << tuple_to_string(t) << '\n';
}

int main()
{
    std::cout << "=== C++26 结构化绑定引入参数包 演示 ===\n\n";

    demo_full_pack(std::tuple{42, 3.14, std::string{"hello"}});
    demo_head_tail(std::tuple{1, 2.5, std::string{"world"}, '!'});
    demo_head_middle_tail(std::tuple{10, 3.14, std::string{"hello"}, 'x', true});
    demo_ignore(std::tuple{100, 2.71, std::string{"ignore_test"}, 'z'});
    demo_dot_product();
    demo_tuple_to_string();

    std::cout << "\n所有演示完成！\n";
    return 0;
}

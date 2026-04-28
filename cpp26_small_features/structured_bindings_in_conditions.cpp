// C++26 Structured Bindings in Conditions - P0963R3
// 演示结构化绑定在条件语句中的使用
// 编译: g++ -std=c++26 -o sb_conditions structured_bindings_in_conditions.cpp && ./sb_conditions

#include <iostream>
#include <optional>
#include <string>
#include <map>
#include <tuple>

// 示例1: 带状态码和错误信息的结果类型
struct Result {
    bool is_successful;
    std::optional<std::string> error_message;

    explicit operator bool() const noexcept {
        return is_successful;
    }
};

Result process(int i) {
    if (i % 2 == 0) {
        return {true, std::nullopt};
    } else {
        return {false, "数值 " + std::to_string(i) + " 不是偶数"};
    }
}

// 示例2: 类似 std::expected 的分组结果
struct DivisionResult {
    bool valid;
    double value;
    std::string description;

    explicit operator bool() const noexcept {
        return valid;
    }
};

DivisionResult safe_divide(double a, double b) {
    if (b == 0) {
        return {false, 0.0, "除以零错误"};
    }
    return {true, a / b, std::to_string(a) + " / " + std::to_string(b)};
}

// 示例3: 查找操作返回多个相关值
struct LookupResult {
    bool found;
    int value;
    std::string source;

    explicit operator bool() const noexcept {
        return found;
    }
};

LookupResult find_in_map(const std::map<std::string, int>& m,
                          const std::string& key) {
    auto it = m.find(key);
    if (it != m.end()) {
        return {true, it->second, "map"};
    }
    return {false, -1, "未找到"};
}

int main()
{
    std::cout << "=== C++26 条件中的结构化绑定 演示 ===\n\n";

    // 示例1: if 语句中的结构化绑定
    std::cout << "--- 示例1: 带错误信息的结果 ---\n";
    for (int n : {1, 2, 4}) {
        // C++26: 在条件中直接使用结构化绑定
        if (const auto& [is_successful, error_message] = process(n)) {
            std::cout << "  处理 " << n << ": 成功\n";
        } else {
            std::cout << "  处理 " << n << ": 失败 - "
                      << error_message.value_or("") << '\n';
        }
    }

    // 示例2: 带描述的计算结果
    std::cout << "\n--- 示例2: 安全除法 ---\n";
    for (auto [a, b] : {std::pair{10.0, 2.0}, {5.0, 0.0}, {7.0, 3.0}}) {
        if (const auto& [valid, val, desc] = safe_divide(a, b)) {
            std::cout << "  " << desc << " = " << val << '\n';
        } else {
            std::cout << "  错误: " << desc << '\n';
        }
    }

    // 示例3: 查找操作
    std::cout << "\n--- 示例3: Map 查找 ---\n";
    std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}};

    for (const auto& name : {"Alice", "Charlie", "Bob"}) {
        // C++26: if 条件中的结构化绑定
        if (const auto& [found, val, src] =
                find_in_map(scores, name)) {
            std::cout << "  找到 " << name << ": " << val
                      << " (来源: " << src << ")\n";
        } else {
            std::cout << "  " << name << " 未找到 (来源: " << src << ")\n";
        }
    }

    return 0;
}

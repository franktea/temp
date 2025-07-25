#include <chrono>
#include <format>
#include <iostream>
#include <ranges>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <iterator>
#include <cctype>
#include <memory>

namespace chrono = std::chrono;
namespace rgs = std::ranges;
namespace vws = std::views;

using sys_days = chrono::sys_days;
using days = chrono::days;
using namespace std::chrono_literals;

// 生成日期序列（从start_year年1月1日到stop_year年1月1日前一天）
auto dates(unsigned start_year, unsigned stop_year) {
    const auto start = sys_days{chrono::year{static_cast<int>(start_year)}/1/1};
    const auto stop = sys_days{chrono::year{static_cast<int>(stop_year)}/1/1};
    auto days_count = (stop - start).count();
    return vws::iota(0LL, days_count)
        | vws::transform([start](long long n) {
            return start + days{n};
        });
}

// 按月份分组
auto by_month() {
    return vws::chunk_by([](sys_days d1, sys_days d2) {
        auto ymd1 = chrono::year_month_day(d1);
        auto ymd2 = chrono::year_month_day(d2);
        return ymd1.month() == ymd2.month() && ymd1.year() == ymd2.year();
    });
}

// 按周分组（周日到周六为一周）
auto by_week() {
    return vws::chunk_by([](sys_days d1, sys_days d2) {
        // 计算两个日期所在的周日
        auto sunday1 = d1 - (chrono::weekday(d1) - chrono::Sunday);
        auto sunday2 = d2 - (chrono::weekday(d2) - chrono::Sunday);
        return sunday1 == sunday2;
    });
}

// 日期格式化（例如："  1"）
auto format_day() {
    return vws::transform([](sys_days d) {
        auto ymd = chrono::year_month_day(d);
        return std::format("{:>3}", static_cast<unsigned>(ymd.day()));
    });
}

// 周格式化（前置空格 + 日期）
auto format_week() {
    return vws::transform([](auto&& week_range) {
        // 获取第一天的星期几
        auto first_day = *rgs::begin(week_range);
        auto wd = chrono::weekday(first_day);
        int spaces = (wd.c_encoding() - chrono::Sunday.c_encoding() + 7) % 7 * 3;
        
        // 格式化为字符串
        std::string week_str;
        for (auto&& day : week_range) {
            week_str += std::format("{:>3}", static_cast<unsigned>(chrono::year_month_day(day).day()));
        }
        
        return std::format("{:<{}}{}", "", spaces, week_str);
    });
}

// 月份标题居中
constexpr std::string_view month_names[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

auto month_title() {
    return vws::transform([](auto&& month_range) {
        auto first_day = *rgs::begin(month_range);
        auto ymd = chrono::year_month_day(first_day);
        auto month_idx = static_cast<unsigned>(ymd.month()) - 1;
        return std::format("{:^22}", month_names[month_idx]);
    });
}

// 布局单个月份（标题 + 最多6周）
auto layout_month() {
    return vws::transform([](auto&& month_range) {
        // 收集周数据
        auto weeks = month_range | by_week() | format_week();
        std::vector<std::string> weeks_vec;
        for (auto&& week : weeks) {
            weeks_vec.push_back(week);
        }
        size_t week_count = weeks_vec.size();
        
        // 标题行
        auto title = month_title()(vws::single(month_range));
        std::string title_str;
        for (auto&& t : title) {
            title_str = t;
            break; // 只有一个元素
        }
        
        // 构建月份行：标题 + 周数据 + 空行（补足到6行）
        std::vector<std::string> lines;
        lines.push_back(title_str);
        for (size_t i = 0; i < std::min(week_count, size_t(6)); ++i) {
            lines.push_back(weeks_vec[i]);
        }
        for (size_t i = week_count; i < 6; ++i) {
            lines.push_back(std::string(22, ' '));
        }
        return lines;
    });
}

// 转置月份组（行转列）
auto transpose_months() {
    return vws::transform([](auto&& months_group) {
        // 收集所有月份的行
        std::vector<std::vector<std::string>> all_lines;
        for (auto&& month : months_group) {
            all_lines.push_back(month); // month 是一个 vector<string>
        }
        
        // 转置：将每个月份的第i行取出来组成新行
        std::vector<std::string> transposed;
        for (size_t i = 0; i < 7; i++) { // 7行：标题+最多6周
            std::string line;
            for (size_t j = 0; j < all_lines.size(); j++) {
                if (i < all_lines[j].size()) {
                    line += all_lines[j][i] + "  ";
                } else {
                    line += std::string(22, ' ') + "  ";
                }
            }
            transposed.push_back(line);
        }
        return transposed;
    });
}

// 连接月份行
auto join_months() {
    return vws::join;
}

// 核心日历格式化管道
auto format_calendar(size_t months_per_line) {
    return by_month()
        | layout_month()
        | vws::chunk(months_per_line)
        | transpose_months()
        | join_months();
}

// 参数解析
struct Config {
    unsigned start = 0;
    unsigned stop = 0;
    size_t per_line = 3;
};

Config parse_args(int argc, char** argv) {
    Config cfg;
    if (argc < 2) {
        throw std::runtime_error("Missing required argument: start year");
    }
    
    cfg.start = std::stoul(argv[1]);
    
    if (argc > 2) {
        cfg.stop = std::stoul(argv[2]);
    } else {
        cfg.stop = cfg.start + 1; // 默认只显示一年
    }
    
    if (argc > 3) {
        cfg.per_line = std::stoul(argv[3]);
        if (cfg.per_line < 1 || cfg.per_line > 6) {
            throw std::runtime_error("Months per line must be between 1 and 6");
        }
    }
    
    // 验证参数
    if (cfg.stop <= cfg.start) {
        throw std::runtime_error("Stop year must be greater than start year");
    }
    
    return cfg;
}

int main(int argc, char** argv) {
    try {
        auto cfg = parse_args(argc, argv);
        
        // 生成日期序列
        auto date_range = dates(cfg.start, cfg.stop);
        
        // 生成并输出日历
        auto calendar_lines = date_range | format_calendar(cfg.per_line);
        
        for (const auto& line : calendar_lines) {
            std::cout << line << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        std::cerr << "Usage: " << argv[0] 
                  << " <start_year> <stop_year> [months_per_line=3]\n";
        return 1;
    }
    return 0;
}
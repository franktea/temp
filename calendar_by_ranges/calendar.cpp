#include <chrono>
#include <format>
#include <iostream>
#include <ranges>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cctype>
#include <sstream>
#include <iterator>
#include <cassert>
#include <iomanip>

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
    return vws::iota(0LL, static_cast<long long>((stop - start).count()))
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
std::string format_day(sys_days d) {
    auto ymd = chrono::year_month_day(d);
    return std::format("{:>3}", static_cast<unsigned>(ymd.day()));
}

// 周格式化（前置空格 + 日期）
std::string format_week(const auto& week_range) {
    // 创建一个向量来存储日期
    std::vector<sys_days> week;
    for (auto&& day : week_range) {
        week.push_back(day);
    }
    
    if (week.empty()) return "";
    
    auto first_day = week.front();
    auto wd = chrono::weekday(first_day);
    int spaces = (wd.c_encoding() - chrono::Sunday.c_encoding() + 7) % 7 * 3;
    std::string day_str;
    for (auto day : week) {
        day_str += format_day(day);
    }
    return std::format("{:<{}}{}", "", spaces, day_str);
}

// 月份标题居中
constexpr std::string_view month_names[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

std::string month_title(sys_days d) {
    auto ymd = chrono::year_month_day(d);
    auto month_idx = static_cast<unsigned>(ymd.month()) - 1;
    return std::format("{:^22}", month_names[month_idx]);
}

// 布局单个月份（标题 + 最多6周）
auto layout_month(const auto& month_range) {
    std::vector<std::string> lines;
    if (rgs::empty(month_range)) return lines;
    
    // 创建月份的日期向量
    std::vector<sys_days> month;
    for (auto&& day : month_range) {
        month.push_back(day);
    }
    
    lines.push_back(month_title(month.front())); // 标题行
    
    // 按周分组并格式化
    auto weeks = month | by_week();
    for (auto&& week : weeks) {
        lines.push_back(format_week(week));
    }
    
    // 补足6行（标题+最多5周数据）
    lines.resize(7, std::string(22, ' ')); 
    return lines;
}

// 格式日历函数（替代视图管道）
template <typename MonthsRange>
std::vector<std::string> format_calendar(MonthsRange&& months_range, size_t months_per_line) {
    std::vector<std::string> result;
    std::vector<std::vector<std::string>> month_lines;
    
    // 为每个月份生成行
    for (auto&& month : months_range) {
        month_lines.push_back(layout_month(month));
    }
    
    if (month_lines.empty()) return result;
    
    // 分组成多行排列
    for (size_t i = 0; i < month_lines.size(); i += months_per_line) {
        // 获取当前行的月份组
        size_t end_idx = std::min(i + months_per_line, month_lines.size());
        
        // 构建7行（标题+周数据）
        for (size_t line = 0; line < 7; line++) {
            std::string merged_line;
            for (size_t col = i; col < end_idx; col++) {
                if (line < month_lines[col].size()) {
                    merged_line += month_lines[col][line] + "  ";
                } else {
                    merged_line += std::string(22, ' ') + "  ";
                }
            }
            result.push_back(merged_line);
        }
        
        // 添加空行分隔月份组
        if (end_idx < month_lines.size()) {
            result.push_back("");
        }
    }
    
    return result;
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
        if (std::string(argv[2]) == "never") {
            cfg.stop = 0; // 标记为无限模式
        } else {
            cfg.stop = std::stoul(argv[2]);
        }
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
    if (cfg.stop != 0 && cfg.stop <= cfg.start) {
        throw std::runtime_error("Stop year must be greater than start year");
    }
    
    return cfg;
}

int main(int argc, char** argv) try {
    Config cfg;
    try {
        cfg = parse_args(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        std::cerr << "Usage: " << argv[0] 
                  << " <start_year> [stop_year|never] [months_per_line=3]\n";
        return 1;
    }
    
    try {
        if (cfg.stop == 0) { // 无限模式
            // 设置一个合理的截止年份
            const unsigned end_year = cfg.start + 10;
            
            // 生成日期序列
            auto date_range = dates(cfg.start, end_year);
            
            // 按月份分组
            auto months = date_range | by_month();
            
            // 格式化为日历行
            auto calendar_lines = format_calendar(months, cfg.per_line);
            
            // 输出结果
            for (const auto& line : calendar_lines) {
                std::cout << line << '\n';
            }
        } else {
            // 生成日期序列
            auto date_range = dates(cfg.start, cfg.stop);
            
            // 按月份分组
            auto months = date_range | by_month();
            
            // 格式化为日历行
            auto calendar_lines = format_calendar(months, cfg.per_line);
            
            // 输出结果
            for (const auto& line : calendar_lines) {
                std::cout << line << '\n';
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Calendar generation error: " << e.what() << '\n';
        return 1;
    }
} catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << '\n';
    return 1;
}

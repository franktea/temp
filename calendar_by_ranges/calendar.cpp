/**
 * @file calendar.cpp
 * @author frankt
 * @brief 
 * @version 0.1
 * @date 2025-07-26
 * 
 * refractoring calendar example from Eric Niebler's range-v3,
 * using std::ranges in C++26.
 * compile with: g++15 -std=c++20 calendar.cpp -o calendar
 * 
 */

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>
#include <chrono>
#include <format>
#include <cctype>
#include <memory>

namespace views = std::ranges::views;
namespace chrono = std::chrono;
using namespace std::chrono_literals;
using namespace std::string_literals;

using date = chrono::sys_days;
using day_t = chrono::days;

// Custom interleave_view with shared_ptr for data lifetime
namespace my_views {
    template <typename Rngs>
    requires std::ranges::forward_range<Rngs> &&
             std::ranges::forward_range<std::ranges::range_value_t<Rngs>>
    class interleave_view : public std::ranges::view_interface<interleave_view<Rngs>> {
    private:
        std::shared_ptr<std::vector<std::ranges::range_value_t<Rngs>>> data_ptr_;
        friend class iterator;
    public:
        interleave_view() = default;
        
        template <typename T>
        explicit interleave_view(T&& rngs)
            : data_ptr_(std::make_shared<std::vector<std::ranges::range_value_t<Rngs>>>(
                  rngs | std::ranges::to<std::vector>()))
        {}

        class iterator {
        public:
            // 关键：正确定义 value_type
            using value_type = std::ranges::range_value_t<std::ranges::range_value_t<Rngs>>;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

        private:
            std::size_t n_ = 0;
            std::vector<std::ranges::iterator_t<std::ranges::range_value_t<Rngs>>> iters_;
            std::vector<std::ranges::sentinel_t<std::ranges::range_value_t<Rngs>>> ends_;
            
        public:
            iterator() = default;
            
            explicit iterator(interleave_view& parent)
            {
                for (auto&& inner_rng : *parent.data_ptr_) {
                    iters_.push_back(std::ranges::begin(inner_rng));
                    ends_.push_back(std::ranges::end(inner_rng));
                }

                // 用这个就知道value_type是啥类型了
                static_assert(std::is_same_v<value_type, std::string>, "value_type must be copyable or movable");
            }

            value_type operator*() const {
                return *iters_[n_];
            }

            iterator& operator++() {
                n_ = (n_ + 1) % iters_.size();
                if (n_ == 0) {
                    for (auto& it : iters_) {
                        ++it;
                    }
                }
                return *this;
            }

            iterator operator++(int) {
                auto tmp = *this;
                ++*this;
                return tmp;
            }

            bool operator==(const iterator&) const = default;

            bool operator==(std::default_sentinel_t) const {
                return iters_.empty() || iters_[0] == ends_[0];
            }
        };
        
        iterator begin() {
            return iterator{*this};
        }
        
        std::default_sentinel_t end() const noexcept {
            return {};
        }
    };

    template <typename Rngs>
    interleave_view(Rngs) -> interleave_view<Rngs>;

    struct interleave_adaptor : std::ranges::range_adaptor_closure<interleave_adaptor> {
        template <std::ranges::viewable_range Rng>
        requires std::ranges::forward_range<std::ranges::range_value_t<Rng>>
        auto operator()(Rng&& rng) const {
            return interleave_view{std::forward<Rng>(rng)};
        }
    };
} // namespace my_views

inline constexpr my_views::interleave_adaptor interleave;

// 日期迭代器，使 sys_days 可以用于 ranges
struct date_iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = date;
    using reference = const date&;
    using pointer = const date*;

    date current;

    date_iterator() = default;
    explicit date_iterator(date d) : current(d) {}
    
    date operator*() const { return current; }
    date operator->() { return current; }
    
    date_iterator& operator++() {
        current += day_t{1};
        return *this;
    }
    
    date_iterator operator++(int) {
        date_iterator tmp = *this;
        ++(*this);
        return tmp;
    }
    
    bool operator==(const date_iterator& rhs) const = default;
};

// 创建日期范围视图
class dates_view : public std::ranges::view_interface<dates_view> {
    date from_;
    date to_;
    
public:
    dates_view() = default;
    dates_view(date from, date to) : from_(from), to_(to) {}
    
    auto begin() const { return date_iterator(from_); }
    auto end() const { return date_iterator(to_); }
    
    std::size_t size() const {
        return (to_ - from_).count();
    }
};

// In: start year, stop year
// Out: range of dates from start to stop
auto dates(unsigned short start, unsigned short stop) {
    auto d_start = chrono::sys_days{chrono::year{start}/1/1};
    auto d_stop = chrono::sys_days{chrono::year{stop}/1/1};
    
    return dates_view(d_start, d_stop);
}
static_assert(std::ranges::forward_range<decltype(dates(2023,2024))>);

// 从指定年份开始的无界日期范围
class dates_from_view : public std::ranges::view_interface<dates_from_view> {
    date from_;
    
public:
    dates_from_view() = default;
    explicit dates_from_view(date from) : from_(from) {}
    
    auto begin() const { return date_iterator(from_); }
    auto end() const { return std::unreachable_sentinel; }
};

// In: year
// Out: infinite range of dates from that year
auto dates_from(unsigned short year) {
    auto d_start = chrono::sys_days{chrono::year{year}/1/1};
    return dates_from_view(d_start);
}
static_assert(std::ranges::forward_range<decltype(dates_from(2023))>);

// In:  range of dates
// Out: range of ranges grouped by month
auto by_month() {
    return views::chunk_by(
        [](date a, date b) { 
            auto a_ym = chrono::year_month_day{a}.year()/chrono::year_month_day{a}.month();
            auto b_ym = chrono::year_month_day{b}.year()/chrono::year_month_day{b}.month();
            return a_ym == b_ym;
        });
}

// In:  range of dates
// Out: range of ranges grouped by ISO week
auto by_week() {
    return views::chunk_by([](date a, date b) {
        return (b - a).count() == 1 && 
               chrono::weekday{a}.c_encoding() != 6;
    });
}

// In:  date object
// Out: formatted day string (right-aligned)
std::string format_day(date d) {
    unsigned day_val = static_cast<unsigned>(chrono::year_month_day{d}.day());
    return std::format("{:>3}", day_val);
}

// In:  range<range<date>> (weeks)
// Out: range<string> formatted weeks
auto format_weeks() {
    return views::transform([](auto&& week) {
        if (std::ranges::empty(week)) {
            return std::string(21, ' ');
        }
        
        auto first_day = *std::ranges::begin(week);
        chrono::weekday wd{first_day};
        
        // 计算前导空格数量（以周日为0）
        int leading_spaces = 3 * wd.c_encoding();
        std::string leading_space_str(leading_spaces, ' ');
        
        // 格式化日期部分
        std::string days_str = week | views::transform(format_day) | views::join | std::ranges::to<std::string>();
        
        return std::format("{:<22}", leading_space_str + days_str);
    });
}

// In:  date
// Out: month title string (centered)
std::string month_title(date d) {
    auto ymd = chrono::year_month_day{d};
    auto month_name = std::format("{:%B}", ymd);
    return std::format("{:^22}", month_name);
}

// In:  range<range<date>> (months)
// Out: range<range<string>> formatted months
auto layout_months() {
    return views::transform([](auto&& month) {
        auto week_count = std::ranges::distance(month | by_week());
        return views::concat(
            views::single(month_title(*std::ranges::begin(month))),
            month | by_week() | format_weeks(),
            views::repeat(std::string(22, ' '), 6 - week_count)
        );
    });
}

// 修改 transpose_closure 实现
struct transpose_closure : std::ranges::range_adaptor_closure<transpose_closure> {
    template <std::ranges::viewable_range Rng>
    requires std::ranges::forward_range<Rng>
    auto operator()(Rng&& rngs) const {
        // 计算输入范围的大小
        auto size = std::ranges::distance(rngs);
        
        // 直接构造 interleave_view
        auto interleaved = my_views::interleave_view{std::forward<Rng>(rngs)};
        
        // 使用双参数版本的 chunk
        return std::ranges::views::chunk(interleaved, size);
    }
};

inline constexpr transpose_closure transpose;

// 修改 transpose_months 函数
auto transpose_months() {
    return views::transform(
        [](auto rng) { return rng | transpose; });
}

// In:  range<range<string>>
// Out: range<string> joined months
auto join_months() {
    return views::transform([](auto rng) {
        return rng | views::join | std::ranges::to<std::string>();
    });
}

// In:  range<date>
// Out: range<string> formatted lines
auto format_calendar(std::size_t months_per_line)
{
    return
        // Group the dates by month:
        by_month()
        // Format the month into a range of strings:
      | layout_months()
        // Group the months that belong side-by-side:
      | views::chunk(months_per_line)
        // Transpose the rows and columns of the size-by-side months:
      | transpose_months()
        // Ungroup the side-by-side months:
      | views::join
        // Join the strings of the transposed months:
      | join_months();
}

int main(int argc, char *argv[]) try {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " start [stop] [--per-line N]\n";
        return 1;
    }
    
    // Parse arguments
    unsigned short start = std::stoi(argv[1]);
    unsigned short stop = start + 1;
    std::size_t months_per_line = 3;
    bool never_stop = false;
    
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "never") {
            never_stop = true;
        } else if (arg == "--per-line") {
            if (++i < argc) months_per_line = std::stoul(argv[i]);
        } else if (std::isdigit(arg[0])) {
            stop = std::stoi(arg);
        }
    }
    
    if (stop <= start) {
        std::cerr << "ERROR: stop year must be > start year\n";
        return 1;
    }
    
    // Generate and print calendar
    if (never_stop) {
        auto cal = dates_from(start) | format_calendar(months_per_line);
        for (auto&& line : cal | views::take(100)) { // Prevent infinite output
            std::cout << line << '\n';
        }
    } else {
        for (auto&& line : dates(start, stop) | format_calendar(months_per_line)) {
            std::cout << line << '\n';
        }
    }
}
catch(std::exception &e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 1;
}
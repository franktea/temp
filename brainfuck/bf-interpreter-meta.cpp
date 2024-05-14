#include <iostream>
#include <array>

// 解释器模板
template<size_t N>
struct BrainfuckInterpreter {
    static constexpr int memorySize = 30000;
    const std::array<char, N> ops;

    constexpr explicit BrainfuckInterpreter(const std::array<char, N> str) :
        ops(str) {}

    constexpr void run() const {
        char memory[memorySize] = {0};
        char* ptr = memory;

        int ip = 0;
        while (ip< N) {
            switch (ops[ip]) {
                case '>': ++ptr; break;
                case '<': --ptr; break;
                case '+': ++*ptr; break;
                case '-': --*ptr; break;
                case '.': std::cout.put(*ptr); break;
                case ',': std::cin >> *ptr; break;
                case '[':
                    if (*ptr == 0) {
                        int depth = 1;
                        while (depth > 0) {
                            ++ip;
                            if (ops[ip] == '[') {
                                ++depth;
                            } else if (ops[ip] == ']') {
                                --depth;
                            }
                        }
                    }
                    break;
                case ']':
                    if (*ptr != 0) {
                        int depth = 1;
                        while (depth > 0) {
                            --ip;
                            if (ops[ip] == '[') {
                                --depth;
                            } else if (ops[ip] == ']') {
                                ++depth;
                            }
                        }
                    }
                    break;
            }
            ++ip;
        }
    }
};

template <size_t N>
constexpr auto makeBrainfuckInterpreter(const char (&str)[N]) {
    std::array<char, N> ops;
    for(size_t i = 0; i < N; ++i) {
        ops[i] = str[i];
    }
    return BrainfuckInterpreter<N>(ops);
}

int main() {
    constexpr auto interpreter = makeBrainfuckInterpreter(">++++++++[<+++++++++>-]<.>++++[<+++++++>-]<+.+++++++..+++.>>++++++[<+++++++>-]<++.------------.>++++++[<+++++++++>-]<+.<.+++.------.--------.>>>++++[<++++++++>-]<+.");
    interpreter.run();
    return 0;
}

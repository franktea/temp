#include<iostream>

// 定义Brainfuck操作
enum BrainfuckOp {
    INC_PTR,
    DEC_PTR,
    INC_VAL,
    DEC_VAL,
    OUTPUT,
    INPUT,
    JUMP_FORWARD,
    JUMP_BACKWARD
};

// 将字符转换为Brainfuck操作
constexpr BrainfuckOp charToOp(char c) {
    switch (c) {
        case '>': return INC_PTR;
        case '<': return DEC_PTR;
        case '+': return INC_VAL;
        case '-': return DEC_VAL;
        case '.': return OUTPUT;
        case ',': return INPUT;
        case '[': return JUMP_FORWARD;
        case ']': return JUMP_BACKWARD;
        default: return static_cast<BrainfuckOp>(-1);
    }
}

// 解释器模板
template<BrainfuckOp... ops>
struct BrainfuckInterpreter {
    static constexpr int memorySize = 30000;
    static constexpr int stackSize = 1000;

    static void run() {
        char memory[memorySize] = {0};
        char* ptr = memory;
        int stack[stackSize] = {0};
        int sp = 0;

        int ip = 0;
        while (ip< sizeof...(ops)) {
            switch (ops[ip]) {
                case INC_PTR: ++ptr; break;
                case DEC_PTR: --ptr; break;
                case INC_VAL: ++*ptr; break;
                case DEC_VAL: --*ptr; break;
                case OUTPUT: std::cout << *ptr; break;
                case INPUT: std::cin >> *ptr; break;
                case JUMP_FORWARD:
                    if (*ptr == 0) {
                        int depth = 1;
                        while (depth > 0) {
                            ++ip;
                            if (ops[ip] == JUMP_FORWARD) {
                                ++depth;
                            } else if (ops[ip] == JUMP_BACKWARD) {
                                --depth;
                            }
                        }
                    }
                    break;
                case JUMP_BACKWARD:
                    if (*ptr != 0) {
                        int depth = 1;
                        while (depth > 0) {
                            --ip;
                            if (ops[ip] == JUMP_FORWARD) {
                                --depth;
                            } else if (ops[ip] == JUMP_BACKWARD) {
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

// 将字符串转换为Brainfuck解释器模板
template <size_t N>
constexpr auto makeBrainfuckInterpreter(const char (&str)[N]) {
    return BrainfuckInterpreter<charToOp(str[0]), charToOp(str[1]), charToOp(str[2]), charToOp(str[3]), charToOp(str[4]), charToOp(str[5]), charToOp(str[6]), charToOp(str[7]), charToOp(str[8]), charToOp(str[9])...>();
}

int main() {
    constexpr auto interpreter = makeBrainfuckInterpreter("++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.<+++++++.--------.+++.------.--------.>+.>++.");
    interpreter.run();
    return 0;
}

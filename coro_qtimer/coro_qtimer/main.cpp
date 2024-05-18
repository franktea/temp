#include <QCoreApplication>
#include <QTimer>
#include <chrono>
#include <iostream>

#if __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
using std::experimental::coroutine_handle;
using std::experimental::suspend_always;
using std::experimental::suspend_never;
#else
#include <corotine>
using std::coroutine_handle;
using std::suspend_always;
using std::suspend_never;
#endif

class TimerAwaitable {
public:
    explicit TimerAwaitable(std::chrono::milliseconds t) : t_(t) {}

    ~TimerAwaitable() {

    }

    bool await_ready() noexcept {
        return t_ <= std::chrono::milliseconds(0);
    }

    void await_suspend(coroutine_handle<> h) noexcept {
        connection_ = QObject::connect(&timer_, &QTimer::timeout,
            [this, h]() mutable {
               QObject::disconnect(connection_);
               h.resume();
            });
        timer_.setSingleShot(true);
        timer_.start(t_);
    }

    void await_resume() noexcept {

    }
private:
    std::chrono::milliseconds t_;
    QTimer timer_;
    QMetaObject::Connection connection_;
};

TimerAwaitable operator co_await(std::chrono::milliseconds t) noexcept {
    return TimerAwaitable(t);
}

struct Task {
    struct promise_type {
        Task get_return_object() { return {}; }
        suspend_never initial_suspend() { return {}; }
        suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
};

Task Test() {
    using namespace std::chrono_literals;

    std::cout<<"test await started.\n";

    co_await 5s;

    std::cout << "test await finished\n";
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Test();

    return a.exec();
}


#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/experimental/co_composed.hpp>
#include <boost/asio/spawn.hpp>

using namespace std;

using boost::asio::awaitable;
using boost::asio::deferred;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::ip::tcp;

template <typename CompletionToken, typename Executor>
auto async_echo(Executor &&executor, const std::string &message,
                const std::string &server, const std::string &port,
                CompletionToken &&token) {
    return boost::asio::async_initiate<CompletionToken, void()>(
        // 无栈协程，实现调用
        boost::asio::experimental::co_composed([&message, &server, &port, &executor](auto state) -> void { 
            tcp::socket s{executor};

            // 解析地址、连接服务器、发送信息、接收信息，全部用无栈协程
            tcp::resolver resolver{executor};
            tcp::resolver::query q{server, port};
            tcp::resolver::iterator ep = co_await resolver.async_resolve(q, deferred);

            co_await s.async_connect(*ep, deferred);

            co_await boost::asio::async_write(s, boost::asio::buffer(message), deferred);

            std::array<char, 1024> b;
            boost::asio::mutable_buffer buffer{b.data(), b.size()};

            const auto bytes = co_await s.async_receive(buffer, deferred);

            assert(bytes == message.size());
            assert(message == string_view(b.data(), bytes));

            co_yield state.complete();
        }), token);
}

int main() {
    try {
        boost::asio::io_context io_context(1);

        // 有栈协程，发起调用
        spawn(io_context, [](boost::asio::yield_context yield) {
            auto executor = get_associated_executor(yield);
            async_echo(executor, "async message 1", "127.0.0.1", "10001", yield);
            async_echo(executor, "async message 2", "127.0.0.1", "10001", yield);
        });

        io_context.run();
    } catch (std::exception &e) {
        clog << "echo Exception: " << e.what() << endl;
    }
}

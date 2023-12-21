
#include <boost/asio.hpp>
#include <cstdio>
#include <iostream>

using namespace std;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::ip::tcp;
namespace this_coro = boost::asio::this_coro;

awaitable<void> echo(tcp::socket socket) {
    cout << "Waiting for message from " << socket.remote_endpoint() << endl;
    try {
        char data[1024];
        for (;;) {
            std::size_t n = co_await socket.async_read_some(
                boost::asio::buffer(data), use_awaitable);
            cout << "* Read message '" << string_view{data, n} << "' from "
                 << socket.remote_endpoint() << endl;
            co_await async_write(socket, boost::asio::buffer(data, n),
                                 use_awaitable);
        }
    } catch (std::exception &e) {
        cout << "echo Exception: " << e.what() << endl;
    }

    cout << "Done with " << socket.remote_endpoint() << endl;
}

awaitable<void> listener() {
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), 10001});
    
    cout << "Entering accept loop" << endl;

    for (;;) {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        cout << "Accepted connection from " << socket.remote_endpoint() << endl;
        co_spawn(executor, echo(std::move(socket)), detached);
    }
}

int main() {
    try {
        boost::asio::io_context io_context(1);

        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });

        co_spawn(io_context, listener(), detached);

        io_context.run();
    } catch (std::exception &e) {
        cout << "Exception: " << e.what() << endl;
    }
}

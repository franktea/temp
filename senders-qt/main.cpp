#include <QCoreApplication>
#include <QTimer>
#include <iostream>
#include "qtstdexec.h"
#include "exec/task.hpp"
#include "exec/async_scope.hpp"

using namespace QtSteExec;

exec::task<void> AsyncSleep() {
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(1000);
    co_await QObjectAsSender(&timer, &QTimer::timeout);
    std::cout<<"time out after 1000 ms\n";
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    exec::async_scope scope;
    auto t = AsyncSleep();
    scope.spawn(std::move(t));
    std::cout<<"start running...\n";

    return a.exec();
}

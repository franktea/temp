#include <QCoreApplication>
#include <QTimer>
#include <iostream>
#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <optional>
#include "qtstdexec.h"
#include "exec/task.hpp"
#include "exec/async_scope.hpp"

using namespace QtSteExec;

exec::task<void> FetchFileLength() {
    std::cout<<"start fetch file...\n";
    QNetworkAccessManager nam;
    QNetworkRequest req = QNetworkRequest(QUrl(QLatin1String("https://ftp.funet.fi/pub/Linux/kernel/v5.x/linux-5.19.tar.gz")));
    QNetworkReply* reply = nam.head(req);
    co_await QObjectAsSender(reply, &QNetworkReply::finished);
    QVariant length = reply->header(QNetworkRequest::ContentLengthHeader);
    std::cout<<"get file info length = "<< length.value<long>() << "\n";
    reply->deleteLater();
}

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
    auto l = FetchFileLength();
    scope.spawn(std::move(l));
    std::cout<<"start running...\n";

    return a.exec();
}

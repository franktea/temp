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

QNetworkRequest MakeReqeust(const QNetworkRequest& req,
    long bytes_transferred, long chunk_size) {
    QString range_template(QLatin1String("bytes=%1-%2"));
    QString ranges_header = range_template.arg(bytes_transferred).arg(bytes_transferred + chunk_size - 1);
    QNetworkRequest ret = req;
    ret.setRawHeader(QByteArray("Range"), ranges_header.toUtf8());
    std::cout<<ranges_header.toStdString()<<"\n";
    return ret;
}

exec::task<void> FetchFileLength() {
    std::cout<<"start fetch file...\n";
    QNetworkAccessManager nam;
    QNetworkRequest req = QNetworkRequest(QUrl(QLatin1String("https://ftp.funet.fi/pub/Linux/kernel/v5.x/linux-5.19.tar.gz")));
    QNetworkReply* reply = nam.head(req);
    co_await QObjectAsSender(reply, &QNetworkReply::finished);
    QVariant length = reply->header(QNetworkRequest::ContentLengthHeader);
    const auto content_length = length.value<long>();
    std::cout<<"get file info length = "<< content_length << "\n";
    reply->deleteLater();

    long bytes_downloaded = 0;
    const long chunk_size = 1000000;
    while(bytes_downloaded < content_length) {
        req = MakeReqeust(req, bytes_downloaded, chunk_size);
        QNetworkReply* reply = nam.get(req);
        qint64 bytes = 0;
        qint64 total = 0;
        do {
            std::tie(bytes, total) = co_await QObjectAsTupleSender(reply, &QNetworkReply::downloadProgress);
            std::cout<<"... bytes = " << bytes << ", total = " << total << "\n";
        } while(bytes != total);
        co_await QObjectAsSender(reply, &QNetworkReply::finished);
        bytes_downloaded += total;
        std::cout<<"dowloaded " << bytes_downloaded << " of " << content_length << "\n";
        reply->deleteLater();
    }
    std::cout<<"finished downloading file\n";
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

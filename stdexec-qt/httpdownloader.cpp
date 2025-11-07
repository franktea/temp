#include "httpdownloader.h"  // 包含头文件

#include <QCoreApplication>  // Qt 核心应用类
#include <QObject>           // Qt 对象基类
#include <QNetworkReply>     // Qt 网络回复
#include <QThread>           // Qt 线程类

#include "qtstdexec.h"       // Qt 与 stdexec 集成头文件
#undef forever               // 取消 Qt 的 forever 宏定义，避免与 stdexec 冲突
#ifndef Q_MOC_RUN            // 防止 MOC 处理 stdexec 头文件
#include <stdexec/execution.hpp>         // stdexec 执行器框架
#include <exec/repeat_effect_until.hpp>  // stdexec 重复效果直到条件满足
#endif

#include <iostream>  // 标准输入输出
#include <memory>    // 智能指针
#include <tuple>     // 元组

using namespace QtStdExec;  // 使用 QtStdExec 命名空间

/**
 * @brief HttpDownloader 构造函数
 *
 * 初始化 HTTP 下载器对象，设置父对象以便 Qt 对象树管理。
 *
 * @param parent 父对象指针
 */
HttpDownloader::HttpDownloader(QObject *parent)
    : QObject{parent}
{
    // 构造函数体，目前没有额外的初始化代码
}

/**
 * @brief HttpDownloader 析构函数
 *
 * 在对象销毁时自动清理异步操作，使用 QAsyncScopeGuard 确保所有异步操作完成。
 * 这避免了在异步操作仍在进行时销毁对象导致的未定义行为。
 */
HttpDownloader::~HttpDownloader()
{
    QAsyncScopeGuard cleanup{async_scope};  // 自动等待所有异步操作完成
}

static void updateContentLength(QNetworkReply* reply, long& contentLength)
{
        QVariant length = reply->header(QNetworkRequest::ContentLengthHeader);
        contentLength = length.value<long>();
        std::cout << "contentLength: " << contentLength  << std::endl;
}

static void updateBytesDownloaded(QNetworkReply* reply, long& bytesDownloaded)
{
        std::cout << "bytesDownloaded before update: " << bytesDownloaded << std::endl;
        QVariant length = reply->header(QNetworkRequest::ContentLengthHeader);
        bytesDownloaded += length.value<long>();
        std::cout << "bytesDownloaded after update: " << bytesDownloaded  << std::endl;
}

static QNetworkRequest setupRequest(const QNetworkRequest& orig,
                                    long bytesTransferred, long chunkSize)
{
    QString rangeTemplate(QLatin1String("bytes=%1-%2"));
    QString rangeHeader = rangeTemplate.arg(bytesTransferred).arg(bytesTransferred + chunkSize - 1);
    QNetworkRequest req = orig;
    req.setRawHeader(QByteArray("Range"), rangeHeader.toUtf8());
    return req;
}

static bool isDownloadComplete(long bytesTransferred, long contentLength)
{
    bool ret = (bytesTransferred == contentLength);
    if (ret)
        std::cout << "All done!" << std::endl;
    return ret;
}

void HttpDownloader::fetchFile()
{
    bytesDownloaded = 0;
    contentLength = 0;
    reportDownloadProgress();

    req = QNetworkRequest(QUrl(QLatin1String("https://ftp.funet.fi/pub/Linux/kernel/v5.x/linux-5.19.tar.gz")));
    QNetworkReply* reply = nam.head(req);
    auto head_sender_via = qObjectAsSender(reply, &QNetworkReply::finished)
            | stdexec::continues_on(qThreadAsScheduler(QCoreApplication::instance()->thread()))
            | stdexec::then([this, reply](){
                updateContentLength(reply, contentLength);
                contentLengthUpdated(contentLength);
                reply->deleteLater();
            });
    auto repeated_get =
        stdexec::let_value(
                stdexec::just(req)
                | stdexec::then([this](QNetworkRequest request) {
                      return setupRequest(request,
                                          bytesDownloaded,
                                          chunkSize);}),
                [this](QNetworkRequest request) {
                    QNetworkReply* get_reply = nam.get(request);
                    return qObjectAsSender(get_reply,
                                           &QNetworkReply::finished)
                    | stdexec::then([get_reply]() {return get_reply;});
                })
        | stdexec::continues_on(
            qThreadAsScheduler(QCoreApplication::instance()->thread()))
            | stdexec::then([this](QNetworkReply* get_reply) {
                    updateBytesDownloaded(get_reply, bytesDownloaded);
                    reportDownloadProgress();
                    get_reply->deleteLater();
              })
        | stdexec::then([this]() {
              return isDownloadComplete(bytesDownloaded, contentLength);
          })
        | exec::repeat_effect_until();
    auto combined = stdexec::let_value(head_sender_via,
                                       [rget = std::move(repeated_get)]
                                       {return rget;})
                    | stdexec::upon_stopped([]{std::cout << "Download pipeline canceled!" << std::endl;});
    async_scope.spawn(std::move(combined));
}

void HttpDownloader::fetchFileWithCoro()
{
    fetchResult.emplace(doFetchWithCoro());
    async_scope.spawn(stdexec::on(qThreadAsScheduler(QCoreApplication::instance()->thread()), std::move(*fetchResult)));
}

void HttpDownloader::fetchFileWithTupleCoro()
{
    fetchResult.emplace(doFetchWithTupleCoro());
    async_scope.spawn(stdexec::on(qThreadAsScheduler(QCoreApplication::instance()->thread()), std::move(*fetchResult)));
}

void HttpDownloader::fetchHeadWithCoro()
{
    fetchHeadResult.emplace(doFetchHeadWithCoro());
    auto with_continuation = std::move(*fetchHeadResult)
            | stdexec::then([this](QNetworkReply* reply) {
                updateContentLength(reply, contentLength);
                contentLengthUpdated(contentLength);
                reply->deleteLater();
    });
    async_scope.spawn(stdexec::on(qThreadAsScheduler(QCoreApplication::instance()->thread()), std::move(with_continuation)));
}

exec::task<void> HttpDownloader::doFetchWithCoro()
{
    // 如果不用coroutine，这两个局部变量都要改成状态机
    bytesDownloaded = 0;
    contentLength = 0;
    reportDownloadProgress();

    req = QNetworkRequest(QUrl(QLatin1String("https://ftp.funet.fi/pub/Linux/kernel/v5.x/linux-5.19.tar.gz")));
    QNetworkReply* reply = nam.head(req);
    // 等待signal QNetworkReply::finished
    co_await qObjectAsSender(reply, &QNetworkReply::finished);

    updateContentLength(reply, contentLength);
    contentLengthUpdated(contentLength);
    reply->deleteLater();

    while (bytesDownloaded != contentLength) {
        req = setupRequest(req, bytesDownloaded, chunkSize);
        QNetworkReply* get_reply = nam.get(req);
        // 等待signal QNetworkReply::finished
        co_await qObjectAsSender(get_reply, &QNetworkReply::finished);
        updateBytesDownloaded(get_reply, bytesDownloaded);
        reportDownloadProgress();
        get_reply->deleteLater();
    }
}

exec::task<void> HttpDownloader::doFetchWithTupleCoro()
{
    bytesDownloaded = 0;
    contentLength = 0;
    reportDownloadProgress();

    req = QNetworkRequest(QUrl(QLatin1String("https://ftp.funet.fi/pub/Linux/kernel/v5.x/linux-5.19.tar.gz")));
    QNetworkReply* reply = nam.head(req);
    co_await qObjectAsSender(reply, &QNetworkReply::finished);

    updateContentLength(reply, contentLength);
    contentLengthUpdated(contentLength);
    reply->deleteLater();

    while (bytesDownloaded != contentLength) {
        req = setupRequest(req, bytesDownloaded, chunkSize);
        QNetworkReply* get_reply = nam.get(req);
        qint64 bytes = 0;
        qint64 total = 0;
        do {
            std::tie(bytes, total) =
                    co_await qObjectAsTupleSender(get_reply,
                    &QNetworkReply::downloadProgress);
            qDebug() << "Got " << bytes << " bytes out of " << total;
        } while (bytes != total);
        co_await qObjectAsSender(get_reply, &QNetworkReply::finished);
        updateBytesDownloaded(get_reply, bytesDownloaded);
        reportDownloadProgress();
        get_reply->deleteLater();
    }

}

void HttpDownloader::fetchFileWithStackfulCoro()
{
    bytesDownloaded = 0;
    contentLength = 0;
    reportDownloadProgress();

    req = QNetworkRequest(QUrl(QLatin1String("https://ftp.funet.fi/pub/Linux/kernel/v5.x/linux-5.19.tar.gz")));
    QNetworkReply* reply = nam.head(req);
    auto res = qEventLoopWait(qObjectAsSender(reply, &QNetworkReply::finished));
    if (!res) {
        std::cout << "qEventLoopWait on QNetworkReply::finished() (HTTP HEAD) aborted, abandoning..." << std::endl;
        return;
    }

    updateContentLength(reply, contentLength);
    contentLengthUpdated(contentLength);
    reply->deleteLater();

    while (bytesDownloaded != contentLength) {
        req = setupRequest(req, bytesDownloaded, chunkSize);
        QNetworkReply* get_reply = nam.get(req);
        auto res = qEventLoopWait(qObjectAsSender(get_reply, &QNetworkReply::finished));
        if (!res) {
            std::cout << "qEventLoopWait on QNetworkReply::finished() (HTTP GET) aborted, abandoning..." << std::endl;
            return;
        }
        updateBytesDownloaded(get_reply, bytesDownloaded);
        reportDownloadProgress();
        get_reply->deleteLater();
    }
}

exec::task<QNetworkReply *> HttpDownloader::doFetchHeadWithCoro()
{
    req = QNetworkRequest(QUrl(QLatin1String("https://ftp.funet.fi/pub/Linux/kernel/v5.x/linux-5.19.tar.gz")));
    QNetworkReply* reply = nam.head(req);
    auto head_sender = qObjectAsSender(reply, &QNetworkReply::finished);
    co_await std::move(head_sender);
    co_return reply;
}

void HttpDownloader::reportDownloadProgress()
{
    if (contentLength > 0) {
        double result = (double)bytesDownloaded / (double) contentLength;
        downloadProgress(result);
    } else {
        downloadProgress(0.0);
    }
}

static auto doRunStackfulCoro(QTimer& another_timer)
{
    return stdexec::schedule(qThreadAsScheduler(QCoreApplication::instance()->thread()))
                   | stdexec::let_value([&another_timer](auto&&...){
                         another_timer.start();
                         return qObjectAsSender(&another_timer, &QTimer::timeout);
                     })
                   | stdexec::then([](auto&&...) {
                         std::cout << "Another timer expired" << std::endl;
                         return 1;
                   });
}

void HttpDownloader::runStackfulCoro()
{
    QTimer another_timer;
    another_timer.setInterval(2000);
    int x = 0;
    for (int i = 0; i < 3; ++i) {
        auto res = qEventLoopWait(doRunStackfulCoro(another_timer));
        if (res && res->index() == 0) {
            x += std::get<1>(std::get<0>(*res));
        } else {
            std::cout << "Timeouts aborted (NOT canceled) " << x << std::endl;
            return;
        }
    }

    std::cout << "After timeouts of another timer(s), our value is " << x << std::endl;
    auto res2 = qEventLoopWait(stdexec::just_stopped());
    if (res2 && res2->index() == 0) {
        std::cout << "Got a result from a just_stopped() with index " << res2->index() << std::endl;
        [&](stdexec::set_stopped_t){}(std::get<0>(std::get<0>(*res2)));
    }
    struct dummy_exception{};
    auto res3 = qEventLoopWait(stdexec::just_error(dummy_exception{}));
    if (res3 && res3->index() == 0) {
        std::cout << "Got a result from a just_error() with index " << res3->index() << std::endl;
        [&](stdexec::set_error_t){}(std::get<0>(std::get<0>(*res3)));
        [&](dummy_exception){}(std::get<1>(std::get<0>(*res3)));
    }
    auto res4 = qEventLoopWait(stdexec::just(42));
    if (res4 && res4->index() == 0) {
        std::cout << "Got a result " << std::get<1>(std::get<0>(*res4)) << " from a just(42) with index " << res4->index() << std::endl;
        [&](stdexec::set_value_t){}(std::get<0>(std::get<0>(*res4)));
    }
    auto res5 = qEventLoopWait(stdexec::just(std::make_unique<int>(666)));
    if (res5 && res5->index() == 0) {
        std::cout << "Got a result " << *std::get<1>(std::get<0>(*res5)) << " from a just(make_unique<int>(42)) with index " << res4->index() << std::endl;
        [&](stdexec::set_value_t){}(std::get<0>(std::get<0>(*res5)));
    }
}

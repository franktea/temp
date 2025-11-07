#ifndef HTTPDOWNLOADER_H
#define HTTPDOWNLOADER_H

#include <QObject>                    // Qt 对象基类
#include <QNetworkAccessManager>      // Qt 网络访问管理器
#include <QNetworkRequest>            // Qt 网络请求
#ifndef Q_MOC_RUN                     // 防止 MOC 处理 stdexec 头文件
#include <exec/async_scope.hpp>       // stdexec 异步作用域
#include <exec/task.hpp>              // stdexec 任务
#endif
#include <optional>                   // 标准可选类型

/**
 * @brief HttpDownloader - HTTP 文件下载器
 *
 * 这个类演示了如何将 stdexec 与 Qt 结合使用，实现异步文件下载功能。
 * 它提供了多种下载方法，展示了不同的异步编程模式。
 */
class HttpDownloader : public QObject
{
    Q_OBJECT  // Qt 元对象系统宏

public:
    // 构造函数
    explicit HttpDownloader(QObject *parent = nullptr);
    
    // 析构函数
    ~HttpDownloader();
    
    // Q_INVOKABLE 方法：可以从 QML 调用的方法
    
    /**
     * @brief 使用 stdexec 管道下载文件
     *
     * 这个方法使用 stdexec 的执行器管道来构建复杂的异步下载流程，
     * 展示了如何组合多个异步操作。
     */
    Q_INVOKABLE void fetchFile();
    
    /**
     * @brief 使用协程下载文件
     *
     * 这个方法使用 C++20 协程和 stdexec 任务来实现异步下载，
     * 代码更直观，类似于同步代码。
     */
    Q_INVOKABLE void fetchFileWithCoro();
    
    /**
     * @brief 使用元组协程下载文件
     *
     * 这个方法使用协程和元组发送器来处理多个参数的情况。
     */
    Q_INVOKABLE void fetchFileWithTupleCoro();
    
    /**
     * @brief 使用栈式协程下载文件
     *
     * 这个方法使用基于事件循环的栈式协程来实现异步下载。
     */
    Q_INVOKABLE void fetchFileWithStackfulCoro();
    
    /**
     * @brief 使用协程获取 HTTP HEAD 信息
     *
     * 这个方法专门用于获取文件的头部信息（如内容长度）。
     */
    Q_INVOKABLE void fetchHeadWithCoro();
    
    /**
     * @brief 运行栈式协程示例
     *
     * 这个方法展示了如何使用栈式协程处理定时器等异步操作。
     */
    Q_INVOKABLE void runStackfulCoro();
    
    // QML 属性：内容长度，当值改变时会发出 contentLengthUpdated 信号
    Q_PROPERTY(long contentLength MEMBER contentLength NOTIFY contentLengthUpdated)

signals:
    // 下载进度信号，参数为进度百分比 (0.0 - 1.0)
    void downloadProgress(double progress);
    
    // 内容长度更新信号，当获取到文件总大小时发出
    void contentLengthUpdated(long contentLength);

private:
    // 私有方法
    
    /**
     * @brief 使用协程执行下载任务
     * @return stdexec 任务对象
     */
    exec::task<void> doFetchWithCoro();
    
    /**
     * @brief 使用元组协程执行下载任务
     * @return stdexec 任务对象
     */
    exec::task<void> doFetchWithTupleCoro();
    
    /**
     * @brief 使用协程获取 HTTP HEAD 信息
     * @return 包含 QNetworkReply 指针的 stdexec 任务
     */
    exec::task<QNetworkReply*> doFetchHeadWithCoro();
    
    /**
     * @brief 报告下载进度
     *
     * 这个方法计算当前下载进度并发出 downloadProgress 信号。
     */
    void reportDownloadProgress();

private:
    // 私有成员变量
    
    exec::async_scope async_scope;                    // stdexec 异步作用域，用于管理异步操作
    QNetworkAccessManager nam;                        // Qt 网络访问管理器
    QNetworkRequest req;                              // 网络请求对象
    std::optional<exec::task<void>> fetchResult;      // 可选的下载任务结果
    std::optional<exec::task<QNetworkReply*>> fetchHeadResult;  // 可选的 HEAD 请求任务结果
    long contentLength = 0;                           // 文件总长度（字节）
    long bytesDownloaded = 0;                         // 已下载字节数
    long chunkSize = 1024000;                         // 分块大小（1MB）
};

#endif // HTTPDOWNLOADER_H

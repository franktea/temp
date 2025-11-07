#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <httpdownloader.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    HttpDownloader downloader;
    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/http_example/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.rootContext()->setContextProperty("httpDownloader", &downloader);
    engine.load(url);

    return app.exec();
}

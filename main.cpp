#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "database.h"
#include "gameboard.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QtQml>


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    DataBase database;
    GameBoard* pBoard = new GameBoard();

    database.connectToDB();
    pBoard->appDb = &database;
    pBoard->refresh();

    engine.rootContext()->setContextProperty("BoardLink", pBoard);
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}

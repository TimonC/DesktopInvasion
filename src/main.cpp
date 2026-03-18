#include <QApplication>
#include <QQmlApplicationEngine>
#include <QLoggingCategory>
#include <QOpenGLContext>
#include <globals.h>
#include <Game.h>
#include <tests.h>

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/SystemTrayIcon.qml")));

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    /* Globals::DEBUG = true; */

    QOpenGLContext context;
    if (context.create()) {
        qDebug() << "OpenGL IS WORKING! Version:"
                 << context.format().majorVersion() << "." << context.format().minorVersion();
        qDebug() << "OpenGL profile:" << context.format().profile();
        qDebug() << "OpenGL renderable:" << context.format().renderableType();
    } else {
        qDebug() << "OpenGL FAILED - using software rendering";
    }

    /* if (!Globals::DEBUG)  loggingCategory::setFilterRules("*.debug=false"); */

    runAllTests();

    Game game;


    return app.exec();
}

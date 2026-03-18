#include <QApplication>
#include <QLoggingCategory>
#include <QOpenGLContext>
#include <globals.h>
#include <Game.h>
#include <tests.h>
#include <QQmlApplicationEngine>
#include <variant_mapper.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    app.setApplicationName("DesktopInvasion");
    /* app.setOrganizationName("DesktopInvasion"); */
    /* app.setOrganizationDomain(""); */

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    /* Globals::DEBUG = true; */
    /* if (!Globals::DEBUG)  loggingCategory::setFilterRules("*.debug=false"); */

    QOpenGLContext context;
    if (context.create()) {
        qDebug() << "OpenGL IS WORKING! Version:"
                 << context.format().majorVersion() << "." << context.format().minorVersion();
        qDebug() << "OpenGL profile:" << context.format().profile();
        qDebug() << "OpenGL renderable:" << context.format().renderableType();
    } else {
        qDebug() << "OpenGL FAILED - using software rendering";
    }


    runAllTests();

    Game game(&engine, nullptr);


    return app.exec();
}

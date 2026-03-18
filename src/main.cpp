#include <QApplication>
#include <QLoggingCategory>
#include <QOpenGLContext>
#include <globals.h>
#include <Game.h>
#include <qnamespace.h>
#include <tests.h>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QQmlContext>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    /* Globals::debug(true); */
    float scale = 1.5;
    float speed = 2;
    Globals::scale(scale);
    Globals::animationSpeed(speed);


    // Load fonts from QRC once at app startup
    int pixelFontId = QFontDatabase::addApplicationFont(":/assets/fonts/PressStart2P-Regular.ttf");
    int dotGothicId = QFontDatabase::addApplicationFont(":/assets/fonts/DotGothic16-Regular.ttf");

    // Get the font family names
    QString pixelFontFamily;
    QString dotGothicFamily;

    if (pixelFontId != -1) {
        QStringList families = QFontDatabase::applicationFontFamilies(pixelFontId);
        if (!families.isEmpty()) {
            pixelFontFamily = families.first();
            qDebug() << "Loaded PressStart2P font:" << pixelFontFamily;
        }
    }

    if (dotGothicId != -1) {
        QStringList families = QFontDatabase::applicationFontFamilies(dotGothicId);
        if (!families.isEmpty()) {
            dotGothicFamily = families.first();
            qDebug() << "Loaded DotGothic16 font:" << dotGothicFamily;
        }
    }

    QQmlApplicationEngine engine;

    // Expose font names to QML
    engine.rootContext()->setContextProperty("pixelFontFamily", pixelFontFamily);
    engine.rootContext()->setContextProperty("dotGothicFontFamily", dotGothicFamily);

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

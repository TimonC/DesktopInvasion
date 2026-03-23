#include <SingleInstanceApplication.h>
#include <QLoggingCategory>
#include <QOpenGLContext>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <Game.h>
#include <PokemonDatabase.h>
#include <qnamespace.h>
#include <tests.h>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QQmlContext>
#include <QTimer>

int main(int argc, char *argv[]) {
    SingleInstanceApplication app(argc, argv, "DesktopInvasion");

    const char* env =  getenv("DOCKER_ENV");
    bool isDev = (env && strcmp(env, "dev") == 0);
    if(!isDev && app.shouldExit()){
        qDebug() << "DesktopInvasion is already running!";
        return 0;
    }

    QCoreApplication::setOrganizationName("DesktopInvasion");
    QCoreApplication::setApplicationName("DesktopInvasion");

    QIcon icon;
    icon = QIcon(":/assets/icon/icon.png");
    app.setWindowIcon(icon);

    QOpenGLContext context;
    if (context.create()) {
        qDebug() << "OpenGL IS WORKING! Version:"
                 << context.format().majorVersion() << "." << context.format().minorVersion();
        qDebug() << "OpenGL profile:" << context.format().profile();
        qDebug() << "OpenGL renderable:" << context.format().renderableType();
    } else {
        qDebug() << "OpenGL FAILED - using software rendering";
    }

    int pixelFontId = QFontDatabase::addApplicationFont(":/assets/fonts/PressStart2P-Regular.ttf");
    int dotGothicId = QFontDatabase::addApplicationFont(":/assets/fonts/DotGothic16-Regular.ttf");

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
    engine.rootContext()->setContextProperty("pixelFontFamily", pixelFontFamily);
    engine.rootContext()->setContextProperty("dotGothicFontFamily", dotGothicFamily);

    int initResult = PokemonDatabase::instance().initialize();
    if (initResult == -1) {
        qCritical() << "Failed to initialize database";
        return 1;
    }

    QObject::connect(&app, &QApplication::aboutToQuit, []() {
        PokemonDatabase::instance().shutdown();
    });

    std::unique_ptr<Game> game = std::make_unique<Game>(nullptr);
    return app.exec();
}

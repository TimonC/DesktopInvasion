#include <QApplication>
#include <QLoggingCategory>
#include <QOpenGLContext>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <globals.h>
#include <Game.h>
#include <PokemonDatabase.h>
#include <qnamespace.h>
#include <tests.h>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QQmlContext>
#include <QTimer>


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
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

    QString dbDir;
    QDir dir(dbDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qCritical() << "Failed to create database directory:" << dbDir;
            return 1;
        }
        qDebug() << "Created database directory:" << dbDir;
    }

    QObject::connect(&app, &QApplication::aboutToQuit, []() {
        PokemonDatabase::instance().shutdown();
    });
    QString dbPath = QDir(dbDir).filePath("pokemon.db");
    if (!PokemonDatabase::instance().initialize(dbPath)) {
        qCritical() << "Failed to initialize database at:" << dbPath;
        return 1;
    }
    qDebug() << "Database initialized successfully";


    bool isDev = (qEnvironmentVariable("DOCKER_ENV") == "dev");
    const bool DOOM_TIMER = isDev && (strcmp(std::getenv("VALGRIND_MODE"), "1") == 0);
    const int DOOM_S = 60;
    if (isDev) qDebug() << "=== RUNNING IN DEV MODE ===";
    if (DOOM_TIMER) {
        qDebug() << "=== VALGRIND DEBUG MODE ===";
        qDebug() << "Game will auto-exit in" << DOOM_S << "seconds";

        QTimer::singleShot(DOOM_S * 1000, [&app]() {
            qDebug() << "=== AUTO-EXIT TIMER FIRED ===";
            qDebug() << DOOM_S << "seconds elapsed - exiting cleanly";
            app.quit();
        });
    }

    std::unique_ptr<Game> game = std::make_unique<Game>(nullptr);
    return app.exec();
}

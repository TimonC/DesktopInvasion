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

int main(int argc, char *argv[]) {
    const char* valgrindMode = std::getenv("VALGRIND_MODE");
    const char* isDev = std::getenv("DOCKER_ENV");


    bool isValgrindMode = (valgrindMode && strcmp(valgrindMode, "1") == 0);
    int timeoutSeconds = 30;

    if (isDev) {
        qDebug() << "=== RUNNING IN DEV MODE ===";

        if (isValgrindMode) {
            qDebug() << "=== VALGRIND DEBUG MODE ENABLED ===";
            qDebug() << "Will auto-exit after" << timeoutSeconds << "seconds";
        }
    }

    QApplication app(argc, argv);
    // Set organization and application name for proper data paths
    QCoreApplication::setOrganizationName("DesktopInvasion");
    QCoreApplication::setApplicationName("DesktopInvasion");

    /* Globals::debug(true); */
    float scale = 2;
    float speed = 2;
    Globals::scale(scale);
    Globals::animationSpeed(speed);
    const bool DOOM_TIMER = isDev && isValgrindMode;
    const int DOOM_S = 10;

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

    QString dbDir;
    QString appDirPath = qEnvironmentVariable("APPDIR");

    if (!appDirPath.isEmpty()) {
        dbDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        qDebug() << "Running from AppImage, using data directory:" << dbDir;
    } else {
        // Running from build directory - use local path for development
        dbDir = QCoreApplication::applicationDirPath();
        qDebug() << "Running from build directory, using local path:" << dbDir;
    }

    // Ensure directory exists
    QDir dir(dbDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qCritical() << "Failed to create database directory:" << dbDir;
            return 1;
        }
        qDebug() << "Created database directory:" << dbDir;
    }

    QString dbPath = dbDir + "/pokemon.db";
    qDebug() << "Initializing database at:" << dbPath;

    // Initialize the database with the proper path
    if (!PokemonDatabase::instance().initialize(dbPath.toStdString())) {
        qCritical() << "Failed to initialize database at:" << dbPath;
        return 1;
    }
    qDebug() << "Database initialized successfully";

    Game *game = new Game(&engine, nullptr);
    // Connect game destruction to app quit
    QObject::connect(game, &QObject::destroyed, &app, &QApplication::quit);

    if (DOOM_TIMER) {
        qDebug() << "=== VALGRIND DEBUG MODE ===";
        qDebug() << "Game will auto-exit in" << DOOM_S << "seconds";

        QTimer::singleShot(DOOM_S * 1000, game, [game]() {
            qDebug() << "=== AUTO-EXIT TIMER FIRED ===";
            qDebug() << DOOM_S << "seconds elapsed - exiting cleanly";
            game->deleteLater();
            PokemonDatabase::instance().shutdown();
        });
    }


    return app.exec();
}

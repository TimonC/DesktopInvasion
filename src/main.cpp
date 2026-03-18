#include <QGuiApplication>
#include <QQmlContext>
#include "InvasionOverlay.h"
#include "PokemonSpawner.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    InvasionOverlay overlay;

    // Configure everything in constructor!
    PokemonSpawner spawner(
        "qrc:/assets/HGSS/PokGen1_transparent_reordered.png", // sprite sheet
        0,    // min row
        19,   // max row (20 different sprites)
        50    // max sprites
    );

    // Use rootContext() directly from QQuickView
    overlay.rootContext()->setContextProperty("pokemonSpawner", &spawner);
    overlay.show();

    // Start spawning from main!
    spawner.startSpawning();

    return app.exec();
}

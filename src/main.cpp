#include <QGuiApplication>
#include <QQmlContext>
#include "InvasionOverlay.h"
#include "PokemonSpawner.h"

int main(int argc, char *argv[]){
    QGuiApplication app(argc, argv);

    InvasionOverlay overlay;

    PokemonSpawner spawner(
        "qrc:/assets/HGSS/PokGen1_transparent_reordered.png", // sprite sheet
        0,
        160,
        50
        );

    overlay.rootContext()->setContextProperty("pokemonSpawner", &spawner);
    overlay.show();

    spawner.startSpawning();

    return app.exec();
}

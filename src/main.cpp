#include <QGuiApplication>
#include <QQmlContext>
/* #include "InvasionOverlay.h" */
/* #include "PokemonSpawner.h" */
#include "PokemonInteractable.h"

int main(int argc, char *argv[]){
    QGuiApplication app(argc, argv);
    PokemonInteractable pokemon;
    /* InvasionOverlay overlay; */

    /* PokemonSpawner spawner( */
    /*     "qrc:/assets/HGSS/PokGen1_transparent_reordered.png", // sprite sheet */
        /* 0, */
        /* 160, */
        /* 50 */
        /* ); */

    /* overlay.rootContext()->setContextProperty("pokemonSpawner", &spawner); */
    /* overlay.show(); */

    /* spawner.startSpawning(); */
    pokemon.show();
    return app.exec();
}

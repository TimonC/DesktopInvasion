#include <QGuiApplication>
#include <QQmlContext>
/* #include "InvasionOverlay.h" */
/* #include "PokemonSpawner.h" */
#include "PokemonInteractable.h"

int main(int argc, char *argv[]){
    QGuiApplication app(argc, argv);
    PokemonInteractable pok1(nullptr, 0);
    PokemonInteractable pok2(nullptr, 1);
    PokemonInteractable pok3(nullptr, 2);
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
    pok1.show();
    pok2.show();
    pok3.show();
    return app.exec();
}

#include <QGuiApplication>
#include "PokemonInteractable.h"

int main(int argc, char *argv[]){
    QGuiApplication app(argc, argv);
    PokemonInteractable pok1(nullptr, 0);
    PokemonInteractable pok2(nullptr, 1);
    PokemonInteractable pok3(nullptr, 2);

    pok1.show();
    pok2.show();
    pok3.show();

    return app.exec();
}

#include <QGuiApplication>
#include "PokemonInteractable.h"
int main(int argc, char *argv[]){
    QGuiApplication app(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    PokemonInteractable pok1(nullptr, std::rand()%100);
    PokemonInteractable pok2(nullptr, random()%100);
    PokemonInteractable pok3(nullptr, random()%120);

    pok1.show();
    pok2.show();
    pok3.show();

    return app.exec();
}

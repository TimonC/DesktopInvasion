#include <QGuiApplication>
#include "WildPokemon.h"
int main(int argc, char *argv[]){
    QGuiApplication app(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    WildPokemon pok1(nullptr, std::rand()%100);
    WildPokemon pok2(nullptr, random()%100);
    WildPokemon pok3(nullptr, random()%120);

    pok1.show();
    pok2.show();
    pok3.show();

    return app.exec();
}

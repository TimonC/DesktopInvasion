#include <QGuiApplication>
#include "WildPokemon.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    WildPokemon pok1(nullptr, std::rand() % 100);
    pok1.show();

    WildPokemon pok2(nullptr, std::rand() % 100);
    pok2.show();

    WildPokemon pok3(nullptr, std::rand() % 100);
    pok3.show();

    /* WildPokemon pok4(nullptr, std::rand() % 100); */
    /* pok4.show(); */
    return app.exec();
}

#include <QGuiApplication>
#include "WildPokemon.h"
#include "Player.h"

std::unique_ptr<Player> player = std::make_unique<Player>();

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    WildPokemon pok1(nullptr, std::rand() % 100);
    pok1.show();

    WildPokemon pok2(nullptr, std::rand() % 100);
    pok2.show();

    WildPokemon pok3(nullptr, std::rand() % 100);
    pok3.show();

    return app.exec();
}

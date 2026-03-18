#include <QGuiApplication>
#include "WildPokemon.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::unique_ptr<WildPokemon> pok1  = std::make_unique<WildPokemon>(nullptr, std::rand() % 100);

    std::unique_ptr<WildPokemon> pok2  = std::make_unique<WildPokemon>(nullptr, std::rand() % 100);

    return app.exec();
}

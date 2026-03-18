#include <QGuiApplication>
#include <QDebug>
#include <memory>
#include <cstdlib>
#include <ctime>
#include "WildPokemon.h"
#include "pokemon_data.h"
#include "globals.h"
#include <QLoggingCategory>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    /* Globals::DEBUG = true; */
    /* if (!Globals::DEBUG) { */
        /* QLoggingCategory::setFilterRules("*.debug=false"); */
    /* } */

const PokemonInfo* p1 = Globals::getRandomPokemon();
    const PokemonInfo* p2 = Globals::getRandomPokemon();
    const PokemonInfo* p3 = Globals::getRandomPokemon();

    std::unique_ptr<WildPokemon> pok1 = std::make_unique<WildPokemon>(p1);
    std::unique_ptr<WildPokemon> pok2 = std::make_unique<WildPokemon>(p2);
    std::unique_ptr<WildPokemon> pok3 = std::make_unique<WildPokemon>(p3);


    return app.exec();
}

#include <QGuiApplication>
#include <QDebug>
#include <cstdlib>
#include <ctime>
#include "WildPokemon.h"
#include "pokemon_data.h"
#include "globals.h"
#include <QLoggingCategory>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    Globals::DEBUG = true;
    /* if (!Globals::DEBUG) { */
        /* QLoggingCategory::setFilterRules("*.debug=false"); */
    /* } */

    const int num_wild = 3;
    std::vector<std::unique_ptr<WildPokemon>> wildPokemon;

    for (int i = 1; i <= num_wild; ++i) {
        const PokemonInfo* pokemonInfo = Globals::getRandomPokemon();
        if (pokemonInfo) {
            wildPokemon.push_back(std::make_unique<WildPokemon>(pokemonInfo));
        }
    }

    qDebug() << "Created" << wildPokemon.size() << "pokemon";
    return app.exec();
}

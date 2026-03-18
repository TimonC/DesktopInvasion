#include <QGuiApplication>
#include <QDebug>
#include <memory>
#include <cstdlib>
#include <ctime>
#include "WildPokemon.h"
#include "pokemon_data.h"  // contains kPokemonList, kPokemonCount
#include <globals.h>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const PokemonInfo* p1 = getRandomPokemon();
    const PokemonInfo* p2 = getRandomPokemon();
    const PokemonInfo* p3 = getRandomPokemon();

    std::unique_ptr<WildPokemon> pok1 = std::make_unique<WildPokemon>(p1);
    /* std::unique_ptr<WildPokemon> pok2 = std::make_unique<WildPokemon>(p2); */
    /* std::unique_ptr<WildPokemon> pok3 = std::make_unique<WildPokemon>(p3); */


    if (p1) qDebug() << "Pokémon 1:" << "#" << p1->pokedexId << "-" << p1->name;
    /* if (p2) qDebug() << "Pokémon 2:" << "#" << p2->pokedexId << "-" << p2->name; */
    /* if (p3) qDebug() << "Pokémon 3:" << "#" << p3->pokedexId << "-" << p3->name; */

    return app.exec();
}

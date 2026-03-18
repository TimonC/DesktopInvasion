#include <QGuiApplication>
#include <QDebug>
#include <cstdlib>
#include <ctime>
#include "WildPokemon.h"
#include "pokemon_data.h"
#include "globals.h"
#include <QLoggingCategory>
#include <QOpenGLContext>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    QOpenGLContext context;
    if (context.create()) {
        qDebug() << "OpenGL IS WORKING! Version:"
                 << context.format().majorVersion() << "." << context.format().minorVersion();
        qDebug() << "OpenGL profile:" << context.format().profile();
        qDebug() << "OpenGL renderable:" << context.format().renderableType();
    } else {
        qDebug() << "OpenGL FAILED - using software rendering";
    }

    Globals::DEBUG = true;
    /* if (!Globals::DEBUG) { */
        /* QLoggingCategory::setFilterRules("*.debug=false"); */
    /* } */

    const int num_wild        = 1;
    const int start_pokedexid = 400;
    std::vector<std::unique_ptr<WildPokemon>> wildPokemon;

    /* const PokemonInfo* pokemonInfo1 = Globals::getPokemonByPokedexId(473); */
    /* wildPokemon.push_back(std::make_unique<WildPokemon>(pokemonInfo1)); */

    for (int i = start_pokedexid; i < num_wild + start_pokedexid; ++i) {
        const PokemonInfo* pokemonInfo = Globals::getRandomPokemon();
        /* const PokemonInfo* pokemonInfo = Globals::getPokemonByPokedexId(i); */
        wildPokemon.push_back(std::make_unique<WildPokemon>(pokemonInfo));
    }

    qDebug() << "Created" << wildPokemon.size() << "pokemon";
    return app.exec();
}

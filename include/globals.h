#ifndef GLOBALS_H
#define GLOBALS_H

#include <QRect>
#include <Player.h>
#include <pokemon_data.h>

namespace Globals {
    extern bool DEBUG;
    extern int SCALE;

    const QRect& screenGeometry();
    Player& getPlayer();
    const PokemonInfo* getRandomPokemon();
    const PokemonInfo* findPokemonByPokedexId(int pokedexId);

    const SpriteInfo* getSpriteInfo(int spriteId, int generation);
    QSize getSpriteSize(int spriteId, int generation);
}

#endif

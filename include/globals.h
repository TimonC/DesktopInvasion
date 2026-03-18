#ifndef GLOBALS_H
#define GLOBALS_H

#include <QRect>
#include <Player.h>
#include <pokemon_data.h>

namespace Globals {
    extern bool DEBUG;
    extern int SCALE;
    extern int POKE_PADDING;

    const QRect& screenGeometry();
    Player& getPlayer();
    const PokemonInfo* getRandomPokemon();
    const PokemonInfo* getPokemonByPokedexId(int pokedexId);

    const SpriteInfo* getSpriteInfo(int spriteId, int generation);
    QSize getSpriteSize(int spriteId, int generation);
}

#endif

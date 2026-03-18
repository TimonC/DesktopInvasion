#ifndef GLOBALS_H
#define GLOBALS_H

#include <pokemon_data.h>
class QRect;
class QSize;

namespace Globals {
    extern bool DEBUG;
    extern int SCALE;
    extern int POKE_PADDING;
    extern const int MAX_POKEDEX_ID;

    const QRect& screenGeometry();
    const PokemonInfo* getPokemonInfo(int pokeDexId = -1);

    const SpriteInfo* getSpriteInfo(int spriteId, int generation);
    QSize getSpriteSize(int spriteId, int generation);
}

#endif

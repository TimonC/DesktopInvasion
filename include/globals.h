#ifndef GLOBALS_H
#define GLOBALS_H

#include <QRect>
#include <Player.h>
#include <pokemon_data.h>

namespace Globals {
    extern bool DEBUG;
    extern int SCALE;
    extern int POKE_PADDING;
    extern const int MAX_POKEDEX_ID;

    const QRect& screenGeometry();
    Player& getPlayer();
    const PokemonInfo* getPokemonInfo(std::optional<int> pokedexId = std::nullopt);

    const SpriteInfo* getSpriteInfo(int spriteId, int generation);
    QSize getSpriteSize(int spriteId, int generation);
}

#endif

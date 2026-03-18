#ifndef POKEMON_DATA_H
#define POKEMON_DATA_H

struct PokemonInfo {
    const int pokedexId;
    const int spriteId;
    const int generation;
    const char* name;
};

struct SpriteInfo{
    const int spriteId;
    const int generation;
    const int max_width;
    const int max_height;
    const int xOffset;
    const int yOffset;
};

extern const PokemonInfo kPokemonList[];
extern const int kPokemonCount;

extern const SpriteInfo kSpriteList[];
extern const int kSpriteCount;

#endif

#ifndef POKEMON_DATA_H
#define POKEMON_DATA_H

struct PokemonInfo {
    int pokedexId;
    int spriteId;
    int generation;
    const char* name;
};

struct SpriteInfo{
    int spriteId;
    int generation;
    int width;
    int height;
    int xOffset;
    int yOffset;
};

extern const PokemonInfo kPokemonList[];
extern const int kPokemonCount;

extern const SpriteInfo kSpriteList[];
extern const int kSpriteCount;

#endif

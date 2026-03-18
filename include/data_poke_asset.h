#ifndef DATA_POKE_ASSET_H
#define DATA_POKE_ASSET_H

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
};

extern const int kVariantList[][2];
extern const int kVariantCount;

extern const PokemonInfo kPokemonList[];
extern const int kPokemonCount;

extern const SpriteInfo kSpriteList[];
extern const int kSpriteCount;


#endif

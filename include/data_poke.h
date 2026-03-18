#ifndef DATA_POKE_H
#define DATA_POKE_H

#include <array>
#include <string>

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

struct Party {
    std::array<int, 6> pokedexIds{-1, -1, -1, -1, -1, -1};
    std::array<int, 6> spriteIds{-1, -1, -1, -1, -1, -1};
    std::array<int, 6> iconIds{-1, -1, -1, -1, -1, -1};
    std::array<int, 6> gens{-1, -1, -1, -1, -1, -1};
    std::array<int, 6> ballIds{-1, -1, -1, -1, -1, -1};
    std::array<std::string, 6> names{"", "", "", "", "", ""};
};

#endif

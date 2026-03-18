#pragma once

struct PokemonInfo {
    int pokedexId;
    int spriteId;
    int generation;
    const char* name;
};

extern const PokemonInfo kPokemonList[];
extern const int kPokemonCount;

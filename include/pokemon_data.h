#ifndef POKEMON_DATA_H
#define POKEMON_DATA_H

struct PokemonInfo {
    int pokedexId;
    int spriteId;
    int generation;
    const char* name;
};

extern const PokemonInfo kPokemonList[];
extern const int kPokemonCount;

#endif

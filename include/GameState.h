#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <string>

enum class StatType {
    HP,
    Attack,
    Defense,
    SpecialAttack,
    SpecialDefense,
    Speed,
    COUNT
};

enum class Nature {
    Hardy, Docile, Serious, Bashful, Quirky,
    Lonely, Adamant, Naughty, Brave,
    Bold, Impish, Lax, Relaxed,
    Modest, Mild, Rash, Quiet,
    Calm, Gentle, Careful, Sassy,
    Timid, Hasty, Jolly, Naive
};

struct PokemonState {
    int _id = -1;
    int pokedex_id = 0;
    int variant_id = 0;
    std::string name = "";

    StatType ivs[6] = {};
    StatType evs[6] = {};
    Nature nature = Nature::Hardy;

    int moves[4] = {0, 0, 0, 0};
    int total_xp = 0;
};

struct GameState {
    int _id = 1;
    int wild_pokemon_id = 0;

    int player_sprite_id = 0;
    std::string name = "Player";
    int party_id[6] = {0, 0, 0, 0, 0, 0};
};

#endif

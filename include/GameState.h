#ifndef GAMESTATE_H
#define GAMESTATE_H

class QPoint;
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

struct PokemonState{
    int _id;
    int pokedex_id;
    int variant_id = 0;
    const char* name;

    StatType ivs[6];
    StatType evs[6];
    Nature nature;

    int moves[4];
    int total_xp;
};


struct GameState{
    const int _id;
    const int wild_pokemon_id = 0; //There is only every one wild pokemon, queryable by '0'

    int player_sprite_id;
    const char* name;
    int party_id[6];
};

#endif

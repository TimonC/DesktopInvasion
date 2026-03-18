#ifndef DATA_POKE_H
#define DATA_POKE_H

#include <data_gamestate.h>

struct EligibleMove {
    int move_id;
    int level;
};
struct EligibleEvolve{
    int pokedex_id;
    int level;
};

struct Poke {
    int pokedex_id;
    const char* name;
    Type types[2];
    int base_stats[6];  // HP, Atk, Def, SpAtk, SpDef, Spd
                        //
    const EligibleMove* eligible_moves;
    int eligible_move_count;

    const EligibleEvolve* eligible_evolves;
    int eligible_evolve_count;
};

extern const Poke* const kPokesByIndex[];

#endif

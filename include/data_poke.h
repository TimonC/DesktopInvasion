#ifndef DATA_POKE_H
#define DATA_POKE_H

#include <PokeTypes.h>

struct EligibleMove {
    const int move_id;
    const int level;
};
struct EligibleEvolve{
    const int pokedex_id;
    const int level;
};

struct Poke {
    const int pokedex_id;
    const char* name;
    const Type types[2];
    const int base_stats[6];  // HP, Atk, Def, SpAtk, SpDef, Spd
    const int catch_rate;
    const EligibleMove* eligible_moves;
    const int eligible_move_count;

    const EligibleEvolve* eligible_evolves;
    const int eligible_evolve_count;
};

extern const Poke* const kPokesByIndex[];

#endif

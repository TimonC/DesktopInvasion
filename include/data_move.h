#ifndef DATA_MOVE_H
#define DATA_MOVE_H

#include <array>
#include <data_gamestate.h>

enum class Ailment{
    Burn, Freeze, Paralysis, Poison, Sleep, Confusion, Null
};
struct Move {
    int id;
    const char* name;
    const char* flavor_text;
    Type type;
    int power;
    int accuracy;
    int priority;

    std::array<int, 5> stat_changes;
    Ailment ailment;
    int min_hits;
    int max_hits;
    int min_turns;
    int max_turns;
    int drain;
    int healing;
    int crit_rate;
    int ailment_chance;
    int flinch_chance;
    int stat_chance;

    const int* learned_by_pokemon;
    int learned_count;
};

// The Python script generates these in data_move.cpp
extern const Move* const kMovesByIndex[];
extern const int kMaxMoveId;
extern const int kMoveCount;


#endif

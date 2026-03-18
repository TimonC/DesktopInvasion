#ifndef DATA_MOVE_H
#define DATA_MOVE_H

#include <array>
#include <PokeTypes.h>

enum class Ailment{
    Burn, Freeze, Paralysis, Poison, Sleep, Confusion, Null
};

enum class MoveCategory{
    PhysicalAtk, SpecialAtk, NonDamaging
};

struct Move {
    const int id;
    const char* name;
    const char* flavor_text;
    const Type type;
    const int power;
    const int accuracy;
    const int priority;
    const MoveCategory category;
    const std::array<int, 5> stat_changes;
    const Ailment ailment;
    const int min_hits;
    const int max_hits;
    const int min_turns;
    const int max_turns;
    const int drain;
    const int healing;
    const int crit_rate;
    const int ailment_chance;
    const int flinch_chance;
    const int stat_chance;
};

// The Python script generates these in data_move.cpp
extern const Move* const kMovesByIndex[];
extern const int kMaxMoveId;
extern const int kMoveCount;

#endif

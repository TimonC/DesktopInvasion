#ifndef DATA_MOVE_H
#define DATA_MOVE_H

#include <array>
#include <PokeTypes.h>

struct Move {
    const int id;
    const char* name;
    const char* flavor_text;
    const Type type;
    const int power;
    const int accuracy;
    const int priority;
    const MoveCategory category;
    const std::array<int, 7> stat_changes; //Atk, Def, SpAtk, SpDef, Speed, Accuracy, Evasion,
    const Ailment ailment;
    const int drain;
    const int healing;
    const int crit_rate;
    const int ailment_chance;
    const int flinch_chance;
    const int stat_chance;
    const int stat_change_target; //0 means affects caster, 1 means affects target, anything else means no effect
};

// The Python script generates these in data_move.cpp
extern const Move* const kMovesByIndex[];
extern const int kMaxMoveId;
extern const int kMoveCount;

extern const int kTmCount;
extern const int kAllTmIds[];

#endif

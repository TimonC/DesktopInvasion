#ifndef DATA_MOVE_H
#define DATA_MOVE_H

#include <array>
#include <data_gamestate.h>

struct Move {
    int id;
    const char* name;
    int accuracy;
    int effect_chance;
    int priority;
    int power;
    Type type;
    std::array<int, 5> stat_changes;
    const char* flavor_text;
    const int* learned_by_pokemon;
    int learned_count;
};

// The Python script generates these in data_move.cpp
extern const Move* const kMovesByIndex[];
extern const int kMaxMoveId;
extern const int kMoveCount;


#endif

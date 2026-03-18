#ifndef DATA_BATTLE_H
#define DATA_BATTLE_H

#include <data_gamestate.h>
#include <variant>
#include <array>
#include <vector>

struct Move {
    int id;
    const char* name;
    int accuracy;
    int effect_chance;
    int priority;
    int power;
    const char* type;
    std::array<int, 5> stat_changes;
    const char* flavor_text;
    std::vector<int> learned_by_pokemon;
};

// The Python script generates these in data_battle.cpp
extern const Move* const kMovesByIndex[];
extern const int kMaxMoveId;
extern const int kMoveCount;

enum class StatusCondition{
    Burn, Paralyze, Freeze, Sleep, Confuse, Seeded, Cursed
};

struct StatChange{
    int statIndex;
    int amount;
};

enum class WeatherCondition{
    Clear, Sandstorm, Rain, Sunny, Hail
};

using Effect = std::variant<StatusCondition, StatChange>;

struct SideEffect{
    Effect effect;
    int probability;
};

enum class MoveCategory {
    Physical, Special, Status
};

struct Static{
    int uid;
    int stats[6];
    Type types[2];
    int moves[4];
};

struct State{
    int currentHealth;
    StatusCondition conditions[10];
};

struct Poke{
    Static pokeStatic;
    State pokeState;
};
#endif

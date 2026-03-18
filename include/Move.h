#ifndef MOVE_H
#define MOVE_H

#include <game_data.h>
#include <variant>

enum class StatusCondition{
    Burn, Paralyze, Freeze, Sleep, Confuse, Seeded, Cursed
};

struct StatChange{
    StatType stat;
    int amount;
};

enum class WeatherCondition{
    Sandstorm, Rain, Sunny, Hail
};


using Effect = std::variant<StatusCondition, StatChange>;

struct SideEffect{
    Effect effect;
    int probability;
};

struct Move{
    int id;
    const char* name;
    Type type;
    int power = 0;
    SideEffect* sideEffects;
    StatChange* statChanges;
};


struct MoveMap{
    int id;
    int* eligible;
};


#endif

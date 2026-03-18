#ifndef MOVE_DATA_H
#define MOVE_DATA_H

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

struct Move{
    int id;
    const char* name;
    Type type;
    int power;
    int accuracy;
    MoveCategory moveCategory;
    SideEffect* sideEffects;
    StatChange* statChanges;
};


struct MoveMap{
    int id;
    int* eligible;
};


#endif

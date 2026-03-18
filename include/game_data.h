#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <cstdint>
#include <variant>

enum class StatType : uint8_t {
    HP, Attack, Defense, SpecialAttack, SpecialDefense, Speed
};

enum class Nature : uint8_t {
    Hardy, Docile, Serious, Bashful, Quirky,
    Lonely, Adamant, Naughty, Brave,
    Bold, Impish, Lax, Relaxed,
    Modest, Mild, Rash, Quiet,
    Calm, Gentle, Careful, Sassy,
    Timid, Hasty, Jolly, Naive
};

enum class Type : uint8_t {
    Normal, Fighting, Flying, Bug,
    Ground, Rock, Poison, Ghost,
    Steel, Fire, Water, Grass,
    Electric, Psychic, Ice, Dragon, Dark
};

enum class StatusCondition : uint8_t {
    Confuse, Burn, Paralyze, Poison, Toxic, Freeze, Sleep, Seeded
};

enum class WeatherCondition : uint8_t {
    Sandstorm, Hail, Sunny, Rain
};

struct StatChange {
    StatType statType;
    int8_t change;
};

struct Move {
    uint16_t index;
    const char* name;
    uint8_t power;
    uint8_t accuracy;
    Type type;
};

using BattleEffect = std::variant<StatChange, StatusCondition, WeatherCondition>;

#endif

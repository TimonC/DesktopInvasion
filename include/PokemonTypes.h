#ifndef POKEMONTYPES_H
#define POKEMONTYPES_H

#include <string>
#include <array>

enum Type {
    Null,
    Normal, Fighting, Flying, Poison, Ground,
    Rock, Bug, Ghost, Steel, Fire,
    Water, Grass, Electric, Psychic, Ice,
    Dragon, Dark
};

enum Nature {
    Hardy, Docile, Serious, Bashful, Quirky,
    Lonely, Adamant, Naughty, Brave,
    Bold, Impish, Lax, Relaxed,
    Modest, Mild, Rash, Quiet,
    Calm, Gentle, Careful, Sassy,
    Timid, Hasty, Jolly, Naive
};

inline const char* TYPE_NAMES[] = {
    "Null",
    "Normal", "Fighting", "Flying", "Poison", "Ground",
    "Rock", "Bug", "Ghost", "Steel", "Fire",
    "Water", "Grass", "Electric", "Psychic", "Ice",
    "Dragon", "Dark"
};

inline const char* NATURE_NAMES[] = {
    "Hardy", "Docile", "Serious", "Bashful", "Quirky",
    "Lonely", "Adamant", "Naughty", "Brave",
    "Bold", "Impish", "Lax", "Relaxed",
    "Modest", "Mild", "Rash", "Quiet",
    "Calm", "Gentle", "Careful", "Sassy",
    "Timid", "Hasty", "Jolly", "Naive"
};

inline std::string typeToString(Type type) {
    int idx = type;
    if (idx >= 0 && idx < 18) return TYPE_NAMES[idx];
    return "Unknown";
}

inline std::string natureToString(Nature nature) {
    int idx = nature;
    if (idx >= 0 && idx < 25) return NATURE_NAMES[idx];
    return "Unknown";
}

inline std::array<int, 5> getNatureMultipliers(Nature nature) {
    // 90 = 0.9x, 100 = 1.0x, 110 = 1.1x
    static const int NATURE_MODS[25][5] = {
        {100, 100, 100, 100, 100},
        {100, 100, 100, 100, 100},
        {100, 100, 100, 100, 100},
        {100, 100, 100, 100, 100},
        {100, 100, 100, 100, 100},
        {110,  90, 100, 100, 100}, // Lonely
        {110, 100,  90, 100, 100}, // Adamant
        {110, 100, 100,  90, 100}, // Naughty
        {110, 100, 100, 100,  90}, // Brave
        { 90, 100, 110, 100, 100}, // Bold
        {100,  90, 110, 100, 100}, // Impish
        {100, 100, 110,  90, 100}, // Lax
        {100, 100, 110, 100,  90}, // Relaxed
        { 90, 110, 100, 100, 100}, // Modest
        {100, 110,  90, 100, 100}, // Mild
        {100, 110, 100,  90, 100}, // Rash
        {100, 110, 100, 100,  90}, // Quiet
        { 90, 100, 100, 110, 100}, // Calm
        {100,  90, 100, 110, 100}, // Gentle
        {100, 100,  90, 110, 100}, // Careful
        {100, 100, 100, 110,  90}, // Sassy
        { 90, 100, 100, 100, 110}, // Timid
        {100,  90, 100, 100, 110}, // Hasty
        {100, 100,  90, 100, 110}, // Jolly
        {100, 100, 100,  90, 110}  // Naive
    };

    int idx = nature;
    if (idx >= 0 && idx < 25) {
        return {NATURE_MODS[idx][0], NATURE_MODS[idx][1], NATURE_MODS[idx][2],
                NATURE_MODS[idx][3], NATURE_MODS[idx][4]};
    }
    return {100, 100, 100, 100, 100};
}

inline int getTypeEffectiveness(Type moveType, Type targetTypes[2]) {
    // Effectiveness × 100: 0, 50, 100, 200
    static const int EFFECTIVENESS[18][18] = {
        // Null row
        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
        // Normal
        {100, 100, 100, 100, 100, 100,  50, 100,   0,  50, 100, 100, 100, 100, 100, 100, 100, 100},
        // Fighting
        {100, 200, 100,  50,  50, 100, 200,  50,   0, 200, 100, 100, 100, 100,  50, 200, 100, 200},
        // Flying
        {100, 100, 200, 100, 100, 100,  50, 200, 100,  50, 100, 100, 200,  50, 100, 100, 100, 100},
        // Poison
        {100, 100, 100, 100,  50,  50,  50, 100,  50,   0, 100, 100, 200, 100, 100, 100, 100, 100},
        // Ground
        {100, 100, 100,   0, 200, 100, 200,  50, 100, 200, 200, 100,  50, 200, 100, 100, 100, 100},
        // Rock
        {100, 100,  50, 200, 100,  50, 100, 200, 100,  50, 200, 100, 100, 100, 100, 200, 100, 100},
        // Bug
        {100, 100,  50,  50,  50, 100, 100, 100,  50,  50,  50, 100, 200, 100, 200, 100, 100, 200},
        // Ghost
        {100,   0, 100, 100, 100, 100, 100, 100, 200, 100, 100, 100, 100, 100, 200, 100, 100,  50},
        // Steel
        {100, 100, 100, 100, 100, 100, 200, 100, 100,  50,  50,  50, 100,  50, 100, 200, 100, 100},
        // Fire
        {100, 100, 100, 100, 100, 100,  50, 200, 100, 200,  50,  50, 200, 100, 100, 200,  50, 100},
        // Water
        {100, 100, 100, 100, 100, 200, 200, 100, 100, 100, 200,  50,  50, 100, 100, 100,  50, 100},
        // Grass
        {100, 100, 100,  50,  50, 200, 200,  50, 100,  50,  50, 200,  50, 100, 100, 100,  50, 100},
        // Electric
        {100, 100, 100, 200, 100,   0, 100, 100, 100, 100, 100, 200,  50,  50, 100, 100,  50, 100},
        // Psychic
        {100, 100, 200, 100, 200, 100, 100, 100, 100,  50, 100, 100, 100, 100,  50, 100, 100,   0},
        // Ice
        {100, 100, 100, 200, 100, 200, 100, 100, 100,  50,  50,  50, 200, 100, 100,  50, 200, 100},
        // Dragon
        {100, 100, 100, 100, 100, 100, 100, 100, 100,  50, 100, 100, 100, 100, 100, 100, 200, 100},
        // Dark
        {100, 100,  50, 100, 100, 100, 100, 100, 200, 100, 100, 100, 100, 100, 200, 100, 100,  50}
    };

    int multiplier = 100;
    int moveIdx = moveType;

    if (targetTypes[0] != Null) {
        int targetIdx = targetTypes[0];
        multiplier = multiplier * EFFECTIVENESS[moveIdx][targetIdx] / 100;
    }

    if (targetTypes[1] != Null) {
        int targetIdx = targetTypes[1];
        multiplier = multiplier * EFFECTIVENESS[moveIdx][targetIdx] / 100;
    }

    return multiplier;
}

#endif

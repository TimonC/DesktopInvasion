#ifndef POKETYPES_H
#define POKETYPES_H

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

enum class Ailment{
    Burn, Freeze, Paralysis, Poison, Toxic, Sleep, Confusion, Null
};

enum class MoveCategory{
    PhysicalAtk, SpecialAtk, NonDamaging
};

namespace PokeTypes{
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

    inline int getTypeEffectiveness(Type moveType, Type targetType1, Type targetType2 = Null) {
        // Effectiveness × 100: 0, 50, 100, 200
        static const int EFFECTIVENESS[18][18] = {
            // Null row (index 0)
            {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
            // Normal (index 1)
            {100, 100, 100, 100, 100, 100,  50, 100,   0,  50, 100, 100, 100, 100, 100, 100, 100, 100},
            // Fighting (index 2)
            {100, 200, 100,  50,  50, 100, 200,  50,   0, 200, 100, 100, 100, 100,  50, 200, 100, 200},
            // Flying (index 3)
            {100, 100, 200, 100, 100, 100,  50, 200, 100,  50, 100, 100, 200,  50, 100, 100, 100, 100},
            // Poison (index 4)
            {100, 100, 100, 100,  50,  50,  50, 100,  50,   0, 100, 100, 200, 100, 100, 100, 100, 100},
            // Ground (index 5)
            {100, 100, 100,   0, 200, 100, 200,  50, 100, 200, 200, 100,  50, 200, 100, 100, 100, 100},
            // Rock (index 6)
            {100, 100,  50, 200, 100,  50, 100, 200, 100,  50, 200, 100, 100, 100, 100, 200, 100, 100},
            // Bug (index 7)
            {100, 100,  50,  50,  50, 100, 100, 100,  50,  50,  50, 100, 200, 100, 200, 100, 100, 200},
            // Ghost (index 8) - Gen 4: Steel resists Ghost
            {100,   0, 100, 100, 100, 100, 100, 100, 200,  50, 100, 100, 100, 100, 200, 100, 100,  50},
            // Steel (index 9) - Gen 4: resists Dark & Ghost
            {100, 100, 100, 100, 100, 100, 200, 100, 100,  50,  50,  50, 100,  50, 100, 200, 100,  50},
            // Fire (index 10)
            {100, 100, 100, 100, 100, 100,  50, 200, 100, 200,  50,  50, 200, 100, 100, 200,  50, 100},
            // Water (index 11)
            {100, 100, 100, 100, 100, 200, 200, 100, 100, 100, 200,  50,  50, 100, 100, 100,  50, 100},
            // Grass (index 12)
            {100, 100, 100,  50,  50, 200, 200,  50, 100,  50,  50, 200,  50, 100, 100, 100,  50, 100},
            // Electric (index 13)
            {100, 100, 100, 200, 100,   0, 100, 100, 100, 100, 100, 200,  50,  50, 100, 100,  50, 100},
            // Psychic (index 14)
            {100, 100, 200, 100, 200, 100, 100, 100, 100,  50, 100, 100, 100, 100,  50, 100, 100,   0},
            // Ice (index 15)
            {100, 100, 100, 200, 100, 200, 100, 100, 100,  50,  50,  50, 200, 100, 100,  50, 200, 100},
            // Dragon (index 16)
            {100, 100, 100, 100, 100, 100, 100, 100, 100,  50, 100, 100, 100, 100, 100, 100, 200, 100},
            // Dark (index 17) - Gen 4: Steel resists Dark
            {100, 100,  50, 100, 100, 100, 100, 100, 200,  50, 100, 100, 100, 100, 200, 100, 100,  50}
        };

        int multiplier = 100;
        int moveIdx = moveType;

        if (targetType1 != Null) {
            int targetIdx = targetType1;
            multiplier = multiplier * EFFECTIVENESS[moveIdx][targetIdx] / 100;
        }

        if (targetType2 != Null) {
            int targetIdx = targetType2;
            multiplier = multiplier * EFFECTIVENESS[moveIdx][targetIdx] / 100;
        }

        return multiplier;
    }
}
#endif

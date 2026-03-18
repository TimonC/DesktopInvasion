#ifndef POKEMONTYPES_H
#define POKEMONTYPES_H

#include <string>
#include <array>

enum class Type {
    Null,
    Normal, Fighting, Flying, Poison, Ground,
    Rock, Bug, Ghost, Steel, Fire,
    Water, Grass, Electric, Psychic, Ice,
    Dragon, Dark
};

enum class Nature {
    Hardy, Docile, Serious, Bashful, Quirky,
    Lonely, Adamant, Naughty, Brave,
    Bold, Impish, Lax, Relaxed,
    Modest, Mild, Rash, Quiet,
    Calm, Gentle, Careful, Sassy,
    Timid, Hasty, Jolly, Naive
};

constexpr const char* TYPE_NAMES[] = {
    "Null",
    "Normal", "Fighting", "Flying", "Poison", "Ground",
    "Rock", "Bug", "Ghost", "Steel", "Fire",
    "Water", "Grass", "Electric", "Psychic", "Ice",
    "Dragon", "Dark"
};

constexpr const char* NATURE_NAMES[] = {
    "Hardy", "Docile", "Serious", "Bashful", "Quirky",
    "Lonely", "Adamant", "Naughty", "Brave",
    "Bold", "Impish", "Lax", "Relaxed",
    "Modest", "Mild", "Rash", "Quiet",
    "Calm", "Gentle", "Careful", "Sassy",
    "Timid", "Hasty", "Jolly", "Naive"
};

inline std::string typeToString(Type type) {
    int idx = static_cast<int>(type);
    if (idx >= 0 && idx < 18) return TYPE_NAMES[idx];
    return "Unknown";
}

inline std::string natureToString(Nature nature) {
    int idx = static_cast<int>(nature);
    if (idx >= 0 && idx < 25) return NATURE_NAMES[idx];
    return "Unknown";
}

inline std::array<float, 5> getNatureMultipliers(Nature nature) {
    static constexpr float NATURE_MODS[25][5] = {
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
        {1.1f, 0.9f, 1.0f, 1.0f, 1.0f},
        {1.1f, 1.0f, 0.9f, 1.0f, 1.0f},
        {1.1f, 1.0f, 1.0f, 0.9f, 1.0f},
        {1.1f, 1.0f, 1.0f, 1.0f, 0.9f},
        {0.9f, 1.0f, 1.1f, 1.0f, 1.0f},
        {1.0f, 0.9f, 1.1f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.1f, 0.9f, 1.0f},
        {1.0f, 1.0f, 1.1f, 1.0f, 0.9f},
        {0.9f, 1.1f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.1f, 0.9f, 1.0f, 1.0f},
        {1.0f, 1.1f, 1.0f, 0.9f, 1.0f},
        {1.0f, 1.1f, 1.0f, 1.0f, 0.9f},
        {0.9f, 1.0f, 1.0f, 1.1f, 1.0f},
        {1.0f, 0.9f, 1.0f, 1.1f, 1.0f},
        {1.0f, 1.0f, 0.9f, 1.1f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.1f, 0.9f},
        {0.9f, 1.0f, 1.0f, 1.0f, 1.1f},
        {1.0f, 0.9f, 1.0f, 1.0f, 1.1f},
        {1.0f, 1.0f, 0.9f, 1.0f, 1.1f},
        {1.0f, 1.0f, 1.0f, 0.9f, 1.1f}
    };

    int idx = static_cast<int>(nature);
    if (idx >= 0 && idx < 25) {
        return {NATURE_MODS[idx][0], NATURE_MODS[idx][1], NATURE_MODS[idx][2],
                NATURE_MODS[idx][3], NATURE_MODS[idx][4]};
    }
    return {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
}

inline float getTypeEffectiveness(Type moveType, Type targetTypes[2]) { //gotta trust the LLMs on this one
    static constexpr float EFFECTIVENESS[18][18] = {
        // Attacking type → Defending type (rows = attacker, columns = defender)
        // Order: Null, Normal, Fighting, Flying, Poison, Ground, Rock, Bug, Ghost, Steel, Fire, Water, Grass, Electric, Psychic, Ice, Dragon, Dark
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // Null (shouldn't be used)
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // Normal
        {1.0f, 2.0f, 1.0f, 0.5f, 0.5f, 1.0f, 2.0f, 0.5f, 0.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 2.0f, 1.0f, 2.0f}, // Fighting
        {1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 0.5f, 2.0f, 1.0f, 0.5f, 1.0f, 1.0f, 2.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f}, // Flying
        {1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // Poison
        {1.0f, 1.0f, 1.0f, 0.0f, 2.0f, 1.0f, 2.0f, 0.5f, 1.0f, 2.0f, 2.0f, 1.0f, 0.5f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // Ground
        {1.0f, 1.0f, 0.5f, 2.0f, 1.0f, 0.5f, 1.0f, 2.0f, 1.0f, 0.5f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f}, // Rock
        {1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 1.0f, 2.0f}, // Bug
        {1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 0.5f}, // Ghost
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 1.0f, 2.0f, 1.0f, 1.0f}, // Steel
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 2.0f, 1.0f, 2.0f, 0.5f, 0.5f, 2.0f, 1.0f, 1.0f, 2.0f, 0.5f, 1.0f}, // Fire
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f, 1.0f, 1.0f, 1.0f, 2.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f}, // Water
        {1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 2.0f, 2.0f, 0.5f, 1.0f, 0.5f, 0.5f, 2.0f, 0.5f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f}, // Grass
        {1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 0.5f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f}, // Electric
        {1.0f, 1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.0f}, // Psychic
        {1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 2.0f, 1.0f, 1.0f, 0.5f, 2.0f, 1.0f}, // Ice
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f}, // Dragon
        {1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 0.5f}  // Dark
    };

    float multiplier = 1.0f;
    int moveIdx = static_cast<int>(moveType);

    if (targetTypes[0] != Type::Null) {
        int targetIdx = static_cast<int>(targetTypes[0]);
        multiplier *= EFFECTIVENESS[moveIdx][targetIdx];
    }

    if (targetTypes[1] != Type::Null) {
        int targetIdx = static_cast<int>(targetTypes[1]);
        multiplier *= EFFECTIVENESS[moveIdx][targetIdx];
    }

    return multiplier;
}

#endif

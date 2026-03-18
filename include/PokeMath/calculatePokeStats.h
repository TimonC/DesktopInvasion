#ifndef CALCULATEPOKESTATS_H
#define CALCULATEPOKESTATS_H

#include <array>

inline int applyStatModifier(int stat, int modifier) {
    if(modifier >= 0){
        return stat * (2 + modifier) / 2;
    } else {
        return stat * 2 / (2 - modifier);
    }
}

inline int calculateStat(int lvl, int base, int iv, int ev) {
    return (lvl * (2 * base + iv + ev / 4)) / 100;
}

inline int calculateHealth(int lvl, int base, int iv, int ev){
    return 10 + lvl + calculateStat(lvl, base, iv, ev);
}
inline std::array<int, 6> calculatePokeStats(
    int lvl,
    const int baseStats[6],
    const int ivs[6],
    const int evs[6],
    const std::array<int, 5>& nature  // Changed to int (90, 100, 110)
) {
    std::array<int, 6> result;

    // HP calculation
    result[0] = calculateHealth(lvl, baseStats[0], ivs[0], evs[0]);


    // Integer math for nature
    result[1] = (nature[0] * (5 + calculateStat(lvl, baseStats[1], ivs[1], evs[1]))) / 100;
    result[2] = (nature[1] * (5 + calculateStat(lvl, baseStats[2], ivs[2], evs[2]))) / 100;
    result[3] = (nature[2] * (5 + calculateStat(lvl, baseStats[3], ivs[3], evs[3]))) / 100;
    result[4] = (nature[3] * (5 + calculateStat(lvl, baseStats[4], ivs[4], evs[4]))) / 100;
    result[5] = (nature[4] * (5 + calculateStat(lvl, baseStats[5], ivs[5], evs[5]))) / 100;

    return result;
}

#endif

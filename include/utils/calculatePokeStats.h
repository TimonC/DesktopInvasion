#ifndef CALCULATEPOKESTATS_H
#define CALCULATEPOKESTATS_H

#include <array>

/* https://bulbapedia.bulbagarden.net/wiki/Stat#Determination_of_stats
    For gen 4  (i.e. gen 3 upwards)
*/

inline constexpr int calculateStat(int lvl, int base, int iv, int ev) noexcept {
    return (lvl * (2 * base + iv + ev / 4)) / 100;
}

inline std::array<int, 6> calculatePokeStats(
    int lvl,
    const int baseStats[6],
    const std::array<int, 6>& ivs,
    const std::array<int, 6>& evs,
    const std::array<float, 5>& nature
) noexcept {
    std::array<int, 6> result;

    // HP calculation
    result[0] = 10 + lvl + calculateStat(lvl, baseStats[0], ivs[0], evs[0]);

    // Unrolled loop (static cast to int floors)
    result[1] = static_cast<int>(calculateStat(lvl, baseStats[1], ivs[1], evs[1]) * nature[0]);
    result[2] = static_cast<int>(calculateStat(lvl, baseStats[2], ivs[2], evs[2]) * nature[1]);
    result[3] = static_cast<int>(calculateStat(lvl, baseStats[3], ivs[3], evs[3]) * nature[2]);
    result[4] = static_cast<int>(calculateStat(lvl, baseStats[4], ivs[4], evs[4]) * nature[3]);
    result[5] = static_cast<int>(calculateStat(lvl, baseStats[5], ivs[5], evs[5]) * nature[4]);
    return result;
}

#endif

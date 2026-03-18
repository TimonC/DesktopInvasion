#ifndef PROCESSCATCHATTEMPT_H
#define PROCESSCATCHATTEMPT_H

#include <cmath>
#include <random>

/* https://bulbapedia.bulbagarden.net/wiki/Catch_rate */

inline bool processShake(float modifiedCatchRate) noexcept {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_int_distribution<int> dist(0, 65535);

    const float threshold = 1048560.0f * std::pow(modifiedCatchRate / 16711680.0f, 0.25f);

    return dist(gen) < static_cast<int>(threshold);
}

inline int processCatchAttempt(
        float HP_max,
        float HP_current,
        float catchRate,
        float bonusBall,
        float bonusStatus
) noexcept {
    const float hpFactor = 1.0f - (2.0f * HP_current) / (3.0f * HP_max);
    const float modifiedCatchRate = hpFactor * catchRate * bonusBall * bonusStatus;

    if (!processShake(modifiedCatchRate)) return 0;
    if (!processShake(modifiedCatchRate)) return 1;
    if (!processShake(modifiedCatchRate)) return 2;
    if (!processShake(modifiedCatchRate)) return 3;
    return 4;
}

#endif

#ifndef PROCESSCATCHATTEMPT_H
#define PROCESSCATCHATTEMPT_H

#include <random>

inline bool processShake(float modifiedCatchRate, std::mt19937& rng) {
    float threshold = 1048560.0f / sqrt(sqrt(16711680.0f / modifiedCatchRate));
    std::uniform_int_distribution<int> dist(0, 65535);
    return dist(rng) < static_cast<int>(threshold);
}

inline int processCatchAttempt(
    std::mt19937& rng,
    int HP_max,
    int HP_current,
    int catchRate,
    int ballMod = 100,
    int statusMod = 100
) {
    float hpFactor = (3.0f * HP_max - 2.0f * HP_current) / (3.0f * HP_max);
    float modifiedCatchRate = hpFactor * catchRate * (ballMod / 100.0f) * (statusMod / 100.0f);

    return 4;
    if (!processShake(modifiedCatchRate, rng)) return 0;
    if (!processShake(modifiedCatchRate, rng)) return 1;
    if (!processShake(modifiedCatchRate, rng)) return 2;
    if (!processShake(modifiedCatchRate, rng)) return 3;
    return 4;
}

#endif

#ifndef POKEMATH_H
#define POKEMATH_H

#include <random>
#include <array>
#include <data_move.h>
#include <PokeTypes.h>
#include <cmath>

namespace PokeMath{

    struct DamageParams {
        int lvl;
        int power;
        int attack;
        int defense;
        int burn = 100;
        int critical = 100;
        int stab = 100;
        int type1 = 100;
        int type2 = 100;
    };

    inline int calculateDamage(const DamageParams& p, std::mt19937& rng) {
        static std::uniform_int_distribution<int> damageRandomDist(85, 100);

        // Combine all percentage modifiers into a single multiplier
        // Use int64_t literals to prevent overflow
        int64_t combinedModifier = static_cast<int64_t>(p.burn) * p.stab * p.type1 * p.type2 * p.critical * damageRandomDist(rng);
        int64_t baseDamage = (2 * p.lvl / 5 + 2) * static_cast<int64_t>(p.power) * p.attack / p.defense;

        // Combine all operations: /50, +2, then apply all modifiers at once
        // Use 64-bit constant to avoid overflow
        constexpr int64_t MODIFIER_DIVISOR = 100LL * 100 * 100 * 100 * 100 * 100;
        int64_t damage = (baseDamage / 50) + 2;
        damage = (damage * combinedModifier) / MODIFIER_DIVISOR;

        return static_cast<int>(damage);
    }

    inline int applyStatModifier(int stat, int modifier) {
        // Precomputed lookup tables for stat modifiers (0 to 6, -6 to 0)
        static constexpr int positiveModifiers[7] = {2, 3, 4, 5, 6, 7, 8}; // numerator values for (2 + modifier)/2
        static constexpr int negativeModifiers[7] = {2, 2, 2, 2, 2, 2, 2}; // numerator values for 2/(2 - modifier)
        static constexpr int negativeDenominators[7] = {2, 3, 4, 5, 6, 7, 8}; // denominator values for 2/(2 - modifier)

        if(modifier >= 0) {
            // Clamp to valid range
            int idx = modifier > 6 ? 6 : modifier;
            return (stat * positiveModifiers[idx]) / 2;
        } else {
            int idx = -modifier > 6 ? 6 : -modifier;
            return (stat * negativeModifiers[idx]) / negativeDenominators[idx];
        }
    }

    inline int calculatePoisonDamage(int totalHealth, int counter = -1){
        if(counter < 0){
            return totalHealth >> 3; // division by 8 using bit shift
        } else {
            return (counter * totalHealth) >> 4; // division by 16 using bit shift
        }
    }

    inline int calculateBurnDamage(int totalHealth){
        return totalHealth >> 3; // division by 8 using bit shift
    }

    inline int calculateParalysisSpeed(int speed){
        return speed >> 1; // division by 2 using bit shift
    }

    inline bool calculateParalysisHit(std::mt19937& rng){
        static std::uniform_int_distribution<int> paralysisDist(0, 3);
        return paralysisDist(rng) != 0; // 75% chance to hit (1 in 4 to fail)
    }

    inline bool calculateConfusionHit(std::mt19937& rng){
        static std::uniform_int_distribution<int> confusionDist(0, 1);
        return confusionDist(rng) == 0; // 50% chance to hit
    }

    inline bool calculateFreezeBreak(std::mt19937& rng){
        static std::uniform_int_distribution<int> freezeDist(0, 4);
        return freezeDist(rng) == 0; // 20% chance to break
    }

    inline int calculateAilmentTurns(Ailment ailment, std::mt19937& rng){
        if(ailment == Ailment::Confusion || ailment == Ailment::Sleep){
            static std::uniform_int_distribution<int> ailmentTurnsDist(2, 5);
            return ailmentTurnsDist(rng);
        }
        return -1;
    }

    // Precomputed power of 0.25 values for common catch rates
    inline bool processShake(float modifiedCatchRate, std::mt19937& rng) {
        // Early exit for impossible catches
        if (modifiedCatchRate <= 0.0f) return false;

        // Use integer math where possible
        constexpr float MAX_CATCH_RATE = 255.0f;
        constexpr float INV_MAX_CATCH_RATE = 1.0f / MAX_CATCH_RATE;

        float a = 65536.0f / std::pow(MAX_CATCH_RATE / modifiedCatchRate, 0.25f);

        static std::uniform_int_distribution<int> shakeDist(0, 65535);
        return shakeDist(rng) < static_cast<int>(a);
    }

    inline int calculateBallShakes(
        std::mt19937& rng,
        int HP_max,
        int HP_current,
        int catchRate,
        int ballMod = 100,
        int statusMod = 100
    ) {
        // Early exit for invalid inputs
        if (HP_max <= 0 || catchRate <= 0) return 0;

        // Use float multiplication instead of repeated division
        constexpr float ONE_THIRD = 1.0f / 3.0f;
        float hpFactor = (3.0f * HP_max - 2.0f * HP_current) * ONE_THIRD / HP_max;

        // Precompute combined modifier
        float combinedMod = (ballMod * statusMod) / 10000.0f;
        float modifiedCatchRate = hpFactor * catchRate * combinedMod;

        // Process shakes with early exit
        if (!processShake(modifiedCatchRate, rng)) return 0;
        if (!processShake(modifiedCatchRate, rng)) return 1;
        if (!processShake(modifiedCatchRate, rng)) return 2;
        if (!processShake(modifiedCatchRate, rng)) return 3;
        return 4;
    }

    // Precompute common calculations
    inline int calculateStat(int lvl, int base, int iv, int ev) {
        return (lvl * (2 * base + iv + (ev >> 2))) / 100; // ev/4 using bit shift
    }

    inline int calculateHealth(int lvl, int base, int iv, int ev){
        return 10 + lvl + calculateStat(lvl, base, iv, ev);
    }

    inline std::array<int, 6> calculatePokeStats(
        int lvl,
        const int baseStats[6],
        const int ivs[6],
        const int evs[6],
        const std::array<int, 5>& nature
        ) {
            std::array<int, 6> result;

            // Health calculation
            result[0] = calculateHealth(lvl, baseStats[0], ivs[0], evs[0]);

            // Precompute base stat calculations to avoid repeated function calls
            int baseStatsWithLevel[5];
            for (int i = 0; i < 5; ++i) {
                baseStatsWithLevel[i] = 5 + calculateStat(lvl, baseStats[i + 1], ivs[i + 1], evs[i + 1]);
            }

            // Apply nature modifiers
            result[1] = (nature[0] * baseStatsWithLevel[0]) / 100;
            result[2] = (nature[1] * baseStatsWithLevel[1]) / 100;
            result[3] = (nature[2] * baseStatsWithLevel[2]) / 100;
            result[4] = (nature[3] * baseStatsWithLevel[3]) / 100;
            result[5] = (nature[4] * baseStatsWithLevel[4]) / 100;

            return result;
        }

    inline bool checkAccuracy(int accuracy, std::mt19937& rng) {
        // Early exit for guaranteed hits
        if (accuracy >= 100) return true;
        if (accuracy <= 0) return false;

        static std::uniform_int_distribution<int> accuracyDist(1, 100);
        return accuracyDist(rng) <= accuracy;
    }

    inline bool checkCriticalHit(int critRate, std::mt19937& rng) {
        // Early exits for guaranteed or impossible crits
        if (critRate >= 4) return true; // Always crit for high enough rate

        static constexpr int denominators[4] = {16, 8, 4, 3}; // For crit rates 0-3
        int denom = critRate < 4 ? denominators[critRate] : 2; // critRate 4+ gives denom 2

        static std::uniform_int_distribution<int> critDist(1, denom);
        return critDist(rng) == 1;
    }

    inline bool checkSecondaryEffect(int chance, std::mt19937& rng) {
        // Early exits
        if (chance >= 100) return true;
        if (chance <= 0) return false;

        static std::uniform_int_distribution<int> effectDist(1, 100);
        return effectDist(rng) <= chance;
    }

    inline bool checkSpeedTie(std::mt19937& rng) {
        static std::uniform_int_distribution<int> speedTieDist(0, 1);
        return speedTieDist(rng) == 0;
    }

    // Precomputed level-up experience table for Medium Fast (cubic growth)
    inline int xpToNextLevel(int currentLevel) {
        if (currentLevel >= 100) return 0;

        // Precompute next level's cube
        int nextLevel = currentLevel + 1;
        return nextLevel * nextLevel * nextLevel - currentLevel * currentLevel * currentLevel;
    }

    inline int calculateExperience(int defeatedLevel, int nrParticipated, int baseXP) {
        if (nrParticipated <= 0) return 0;

        // Precompute constant factors
        constexpr float TRAINER_MULTIPLIER = 1.5f;
        constexpr float PARTICIPANT_DIVISOR = 7.0f;

        // Use integer arithmetic where possible
        return static_cast<int>((baseXP * defeatedLevel * TRAINER_MULTIPLIER) /
                                (PARTICIPANT_DIVISOR * nrParticipated));
    }
}

#endif

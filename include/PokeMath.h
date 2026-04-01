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

// https://bulbapedia.bulbagarden.net/wiki/Damage (modified, not all parameters are included in DesktopInvasion)
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

        return static_cast<int>(std::max<int64_t>(1,damage));
    }

// https://bulbapedia.bulbagarden.net/wiki/Stat_modifier#Stage_multipliers
    inline int applyStatModifier(int stat, int modifier) {
        static constexpr int numerators[13] = {
            2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8
        };
        static constexpr int denominators[13] = {
            8, 7, 6, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2
        };

        int idx = modifier + 6;
        return (stat * numerators[idx]) / denominators[idx];
    }

// https://bulbapedia.bulbagarden.net/wiki/Poison_(status_condition)#Effect
  inline int calculatePoisonDamage(int totalHealth, int counter = -1){
        if(counter < 0){
            return totalHealth >> 3; // division by 8 using bit shift
        } else {
            return (counter * totalHealth) >> 4; // division by 16 using bit shift
        }
    }

// https://bulbapedia.bulbagarden.net/wiki/Burn_(status_condition)#Effect
    inline int calculateBurnDamage(int totalHealth){
        return totalHealth >> 3; // division by 8 using bit shift
    }

// https://bulbapedia.bulbagarden.net/wiki/Paralysis_(status_condition)#Effect
    inline int calculateParalysisSpeed(int speed){
        return speed >> 1; // division by 2 using bit shift
    }
    inline bool calculateParalysisHit(std::mt19937& rng){
        static std::uniform_int_distribution<int> paralysisDist(0, 3);
        return paralysisDist(rng) != 0; // 75% chance to hit (1 in 4 to fail)
    }

// https://bulbapedia.bulbagarden.net/wiki/Confusion_(status_condition)#Effect
    inline bool calculateConfusionHit(std::mt19937& rng){
        static std::uniform_int_distribution<int> confusionDist(0, 1);
        return confusionDist(rng) == 0; // 50% chance to hit
    }

// https://bulbapedia.bulbagarden.net/wiki/Freeze_(status_condition)#Effect
    inline bool calculateFreezeBreak(std::mt19937& rng){
        static std::uniform_int_distribution<int> freezeDist(0, 4);
        return freezeDist(rng) == 0; // 20% chance to break
    }

// https://bulbapedia.bulbagarden.net/wiki/Sleep_(status_condition)#Effect
    inline int calculateAilmentTurns(Ailment ailment, std::mt19937& rng){
        if(ailment == Ailment::Confusion || ailment == Ailment::Sleep){
            static std::uniform_int_distribution<int> ailmentTurnsDist(2, 5);
            return ailmentTurnsDist(rng);
        }
        return -1;
    }

// https://bulbapedia.bulbagarden.net/wiki/Catch_rate#Capture_method_(Generation_III-IV)
    inline uint32_t computeShakeThreshold(uint32_t a) {
        if (a == 0) return 0;
        if (a >= 255) return 65536; // Guaranteed

        float invFourthRoot = std::sqrt(std::sqrt(255.0f / a));
        return static_cast<uint32_t>(65536.0f / invFourthRoot);
    }

    inline bool checkShake(uint32_t threshold, std::mt19937& rng) {
        static std::uniform_int_distribution<uint32_t> dist(0, 65535);
        return dist(rng) < threshold;
    }

    inline int calculateBallShakes(
        std::mt19937& rng,
        int HP_max,
        int HP_current,
        int catchRate,
        int ballMod = 100,
        int statusMod = 100
    ) {
        if (ballMod>=10000 || ballMod == 255) return 4;

        if (HP_max <= 0 || catchRate <= 0) return 0;

        // Calculate modified catch rate 'a'
        int hpFactor = 3 * HP_max - 2 * HP_current;
        uint32_t numerator = hpFactor * catchRate * ballMod * statusMod;
        uint32_t denominator = 3 * HP_max * 10000;

        uint32_t a = numerator / denominator;
        if (a > 255) a = 255;

        uint32_t threshold = computeShakeThreshold(a);
        if (threshold == 0) return 0;
        if (threshold == 65536) return 4; // Guaranteed 4 shakes

        if (!checkShake(threshold, rng)) return 0;
        if (!checkShake(threshold, rng)) return 1;
        if (!checkShake(threshold, rng)) return 2;
        if (!checkShake(threshold, rng)) return 3;
        return 4;
    }

// https://bulbapedia.bulbagarden.net/wiki/Stat#Generation_III_onward
    inline int calculateStat(int lvl, int base, int iv=32) {
        return 5 + (lvl * (2 * base + iv)) / 100; // ev/4 using bit shift
    }

    inline int calculateHealth(int lvl, int base, int iv=32){
        return 10 + lvl + (lvl * (2 * base + iv)) / 100;
    }

    inline std::array<int, 6> calculatePokeStats(
        int lvl,
        const int baseStats[6],
        const std::array<int, 5>& nature
        ) {
            std::array<int, 6> result;

            result[0] = calculateHealth(lvl, baseStats[0]);
            int baseStatsWithLevel[5];
            for (int i = 0; i < 5; ++i) {
                baseStatsWithLevel[i] = calculateStat(lvl, baseStats[i + 1]);
            }

            // Apply nature modifiers
            result[1] = (nature[0] * baseStatsWithLevel[0]) / 100;
            result[2] = (nature[1] * baseStatsWithLevel[1]) / 100;
            result[3] = (nature[2] * baseStatsWithLevel[2]) / 100;
            result[4] = (nature[3] * baseStatsWithLevel[3]) / 100;
            result[5] = (nature[4] * baseStatsWithLevel[4]) / 100;

            return result;
        }


// https://bulbapedia.bulbagarden.net/wiki/Accuracy#Accuracy_check
// https://bulbapedia.bulbagarden.net/wiki/Stat_modifier#Stage_multipliers
    inline int calculateModifiedAccuracy(int accuracy, int modifier){
        static constexpr int numerators[13] = {
            33, 36, 43, 50, 60, 75, 100, 133, 166, 200, 233, 266, 300
        };
        int idx = modifier + 6;
        return (accuracy*numerators[idx])/100;
    }

    inline bool checkAccuracy(int accuracy, int modifier, std::mt19937& rng) {
        int modifiedAccuracy = calculateModifiedAccuracy(accuracy, modifier);
        if (modifiedAccuracy >= 100) return true;
        if (modifiedAccuracy <= 0) return false;
        static std::uniform_int_distribution<int> accuracyDist(1, 100);
        return accuracyDist(rng) <= modifiedAccuracy;
    }

//https://bulbapedia.bulbagarden.net/wiki/Giga_Drain_(move)#Generation_II
    inline int calculateDrain(int damage, int drainRate){
        return std::max(1, damage * drainRate / 100);
    }

//https://bulbapedia.bulbagarden.net/wiki/Recover_(move)
    inline int calculateHeal(int totalHealth, int healRate){
        return totalHealth *  healRate / 100;
    }

// https://bulbapedia.bulbagarden.net/wiki/Critical_hit#Generation_II_onwards
    inline bool checkCriticalHit(int critRate, std::mt19937& rng) {
        if (critRate >= 2) return true;

        static constexpr int chances[2] = {16, 8};
        static std::uniform_int_distribution<int> dist(1, chances[critRate]);
        return dist(rng) == 1;
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

// https://bulbapedia.bulbagarden.net/wiki/Experience#Medium_Fast
    inline int xpToNextLevel(int currentLevel) {
        if (currentLevel >= 100) return 0;

        // Precompute next level's cube
        int nextLevel = currentLevel + 1;
        return nextLevel * nextLevel * nextLevel - currentLevel * currentLevel * currentLevel;
    }

// https://bulbapedia.bulbagarden.net/wiki/Experience#Gain_formula
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

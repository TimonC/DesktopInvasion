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
        int random = damageRandomDist(rng);

        int damage = (2 * p.lvl / 5 + 2) * p.power * p.attack / p.defense;
        damage = damage / 50;
        damage = (damage * p.burn) / 100;
        damage = damage + 2;
        damage = (damage * p.stab) / 100;
        damage = (damage * p.type1) / 100;
        damage = (damage * p.type2) / 100;
        damage = (damage * p.critical) / 100;
        damage = (damage * random) / 100;

        return damage;
    }

    inline int applyStatModifier(int stat, int modifier) {
        if(modifier >= 0){
            return stat * (2 + modifier) / 2;
        } else {
            return stat * 2 / (2 - modifier);
        }
    }

    inline int calculatePoisonDamage(int totalHealth, int counter = -1){
        if(counter<0){
            return totalHealth / 8;
        }else{
            return counter * totalHealth / 16;
        }
    }

    inline int calculateBurnDamage(int totalHealth){
        return totalHealth / 8;
    }

    inline int calculateParalysisSpeed(int speed){
        return speed / 2;
    }

    inline bool calculateParalysisHit(std::mt19937& rng){
        static std::uniform_int_distribution<int> paralysisDist(0, 3);
        return paralysisDist(rng) == 0;
    }

    inline bool calculateConfusionHit(std::mt19937& rng){
        static std::uniform_int_distribution<int> confusionDist(0, 1);
        return confusionDist(rng) == 0;
    }

    inline bool calculateFreezeThaw(std::mt19937& rng){
        static std::uniform_int_distribution<int> freezeDist(0, 4);
        return freezeDist(rng) == 0;
    }

    inline int calculateAilmentTurns(Ailment ailment, std::mt19937& rng){
        if(ailment==Ailment::Confusion || ailment==Ailment::Sleep){
            static std::uniform_int_distribution<int> ailmentTurnsDist(2, 5);
            return ailmentTurnsDist(rng);
        }
        return -1;
    }

    // https://bulbapedia.bulbagarden.net/wiki/Catch_rate#Capture_method_(Generation_III-IV)
    inline bool processShake(float modifiedCatchRate, std::mt19937& rng) {
        float a = 65536.0f / powf(255.0f / modifiedCatchRate, 0.25f);

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
        float hpFactor = (3.0f * HP_max - 2.0f * HP_current) / (3.0f * HP_max);
        float modifiedCatchRate = hpFactor * catchRate * (ballMod / 100.0f) * (statusMod / 100.0f);

        if (!processShake(modifiedCatchRate, rng)) return 0;
        if (!processShake(modifiedCatchRate, rng)) return 1;
        if (!processShake(modifiedCatchRate, rng)) return 2;
        if (!processShake(modifiedCatchRate, rng)) return 3;
        return 4;
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
        const std::array<int, 5>& nature
        ) {
            std::array<int, 6> result;

            result[0] = calculateHealth(lvl, baseStats[0], ivs[0], evs[0]);

            result[1] = (nature[0] * (5 + calculateStat(lvl, baseStats[1], ivs[1], evs[1]))) / 100;
            result[2] = (nature[1] * (5 + calculateStat(lvl, baseStats[2], ivs[2], evs[2]))) / 100;
            result[3] = (nature[2] * (5 + calculateStat(lvl, baseStats[3], ivs[3], evs[3]))) / 100;
            result[4] = (nature[3] * (5 + calculateStat(lvl, baseStats[4], ivs[4], evs[4]))) / 100;
            result[5] = (nature[4] * (5 + calculateStat(lvl, baseStats[5], ivs[5], evs[5]))) / 100;

            return result;
        }

    inline bool checkAccuracy(int accuracy, std::mt19937& rng) {
        static std::uniform_int_distribution<int> accuracyDist(1, 100);
        return accuracyDist(rng) <= accuracy;
    }

    inline bool checkCriticalHit(int critRate, std::mt19937& rng) {
        static std::uniform_int_distribution<int> critDist(1, 16/(1+critRate));
        return critDist(rng) == 1;
    }

    inline bool checkSecondaryEffect(int chance, std::mt19937& rng) {
        static std::uniform_int_distribution<int> effectDist(1, 100);
        return effectDist(rng) <= chance;
    }

    inline bool checkSpeedTie(std::mt19937& rng) {
        static std::uniform_int_distribution<int> speedTieDist(0, 1);
        return speedTieDist(rng) == 0;
    }
}

#endif

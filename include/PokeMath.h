#ifndef POKEMATH_H
#define POKEMATH_H

#include <random>
#include <array>
#include <data_move.h>

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
        std::uniform_int_distribution<int> dist(85, 100);
        int random = dist(rng);

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
        return speed / 4;
    }

    inline bool calculateParalysisHit(std::mt19937& rng){
        std::uniform_int_distribution<int> dist(0,3);
        return dist(rng)==0;
    }

    inline bool calculateConfusionHit(std::mt19937& rng){
        std::uniform_int_distribution<int> dist(0,1);
        return dist(rng)==0;
    }

    inline bool calculateFreezeThaw(std::mt19937& rng){
        std::uniform_int_distribution<int> dist(0,4);
        return dist(rng)==0;
    }

    inline int calculateAilmentTurns(Ailment ailment, std::mt19937& rng){
        if(ailment==Ailment::Confusion || ailment==Ailment::Sleep){
            std::uniform_int_distribution<int> dist(2,5);
            return dist(rng);
        }
        return -1;
    }

    inline bool processShake(float modifiedCatchRate, std::mt19937& rng) {
        float threshold = 1048560.0f / sqrt(sqrt(16711680.0f / modifiedCatchRate));
        std::uniform_int_distribution<int> dist(0, 65535);
        return dist(rng) < static_cast<int>(threshold);
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

        return 4;
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
    }

#endif

#ifndef CALCULATEDAMAGE_H
#define CALCULATEDAMAGE_H

#include <random>

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

    return damage > 0 ? damage : 1;
}

#endif

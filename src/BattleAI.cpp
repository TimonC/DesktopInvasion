#include "BattleAI.h"
#include "PokeMath.h"
#include "PokeTypes.h"
#include <cmath>
#include <algorithm>

BattleAI::BattleAI(std::mt19937& rng) : m_rng(rng) {}

int BattleAI::selectMove(const Battler& opponent, const Battler& player) {
    int bestScore = -1000000;
    int bestIndex = 0;
    for (int i = 0; i < 4; ++i) {
        const Move* _move = opponent.pokeState.moves[i];
        if (!_move) continue;
        int score = evaluateMove(_move, opponent, player);
        if (score > bestScore) {
            bestScore = score;
            bestIndex = i;
        }
    }
    return bestIndex;
}

int BattleAI::evaluateMove(const Move* _move, const Battler& caster, const Battler& target) {
    int score = 0;
    score += flag0BadMove(_move, caster, target);
    score += flag1TryToFaint(_move, caster, target);
    score += flag2Viability(_move, caster, target);
    return score;
}

int BattleAI::flag0BadMove(const Move* _move, const Battler& caster, const Battler& target) {
    // Status moves
    if (_move->category == MoveCategory::NonDamaging) {
        // Sleep
        if (_move->ailment == Ailment::Sleep) {
            if (target.battleState.statusCondition == Ailment::Sleep)
                return -10;
        }
        // Poison / Toxic
        if (_move->ailment == Ailment::Poison || _move->ailment == Ailment::Toxic) {
            if (target.battleState.statusCondition == Ailment::Poison ||
                target.battleState.statusCondition == Ailment::Toxic)
                return -10;
        }
        // Paralysis
        if (_move->ailment == Ailment::Paralysis) {
            if (target.battleState.statusCondition == Ailment::Paralysis)
                return -10;
        }
        // Burn
        if (_move->ailment == Ailment::Burn) {
            if (target.battleState.statusCondition == Ailment::Burn)
                return -10;
        }
        // Confusion
        if (_move->ailment == Ailment::Confusion) {
            if (target.battleState.confused == Ailment::Confusion)
                return -5;
        }

        // Stat boosts (positive changes to caster)
        for (int i = 0; i < 7; ++i) {
            if (_move->stat_changes[i] > 0) {
                if (caster.battleState.statModifiers[i] >= 6)
                    return -10;
            }
        }

        // Stat drops (negative changes to target)
        for (int i = 0; i < 7; ++i) {
            if (_move->stat_changes[i] < 0) {
                if (target.battleState.statModifiers[i] <= -6)
                    return -10;
            }
        }
    }

    // Damaging moves – check type immunity
    if (_move->category != MoveCategory::NonDamaging) {
        if (PokeTypes::getTypeEffectiveness(_move->type,
                target.pokeState.types[0] ? *target.pokeState.types[0] : Type::Null,
                target.pokeState.types[1] ? *target.pokeState.types[1] : Type::Null) == 0) {
            return -10;
        }
    }

    return 0;
}

int BattleAI::flag1TryToFaint(const Move* _move, const Battler& caster, const Battler& target) {
    int score = 0;

    // Can faint?
    if (canFaint(_move, caster, target))
        score += 4;
    else
        score += 2;

    // Priority
    if (_move->priority > 0)
        score += 6;
    else
        score += 4;

    // 4x effective bonus
    double eff = getEffectiveness(_move, target);
    if (eff >= 3.9 && random255() < 176)  // 176/256 ≈ 68.75%
        score += 2;

    // ---- Damage comparison ----
    // Find the highest damage among all damaging moves the caster knows
    int highestDamage = 0;
    for (int i = 0; i < 4; ++i) {
        const Move* otherMove = caster.pokeState.moves[i];
        if (!otherMove) continue;
        if (otherMove->category == MoveCategory::NonDamaging) continue;
        int dmg = estimateDamage(otherMove, caster, target);
        if (dmg > highestDamage)
            highestDamage = dmg;
    }
    int myDamage = estimateDamage(_move, caster, target);
    if (myDamage < highestDamage)
        score -= 1;   // Penalty if not the strongest damaging move

    return score;
}

int BattleAI::flag2Viability(const Move* _move, const Battler& caster, const Battler& target) {
    int score = 0;

    // Helper to get HP percent
    auto hpPercent = [](const Battler& b) -> int {
        return b.battleState.currentHealth * 100 / b.pokeState.stats[0];
    };

    // Helper to get speed with modifiers
    auto effectiveSpeed = [](const Battler& b) -> int {
        int spd = b.pokeState.stats[4];
        int mod = b.battleState.statModifiers[4];
        spd = PokeMath::applyStatModifier(spd, mod);
        if (b.battleState.statusCondition == Ailment::Paralysis)
            spd = PokeMath::calculateParalysisSpeed(spd);
        return spd;
    };

    if (_move->category == MoveCategory::NonDamaging) {
        // ---------- Sleep ----------
        if (_move->ailment == Ailment::Sleep) {
            // no score change by default
        }
        // ---------- Confusion ----------
        else if (_move->ailment == Ailment::Confusion) {
            int hp = hpPercent(target);
            if (hp > 70)
                score += 0;
            else if (hp > 50)
                score += (random255() < 128 ? -1 : 0);
            else if (hp > 30)
                score += (random255() < 128 ? -2 : -1);
            else
                score += (random255() < 128 ? -3 : -2);
        }
        // ---------- Paralysis ----------
        else if (_move->ailment == Ailment::Paralysis) {
            int targetSpd = effectiveSpeed(target);
            int casterSpd = effectiveSpeed(caster);
            if (targetSpd > casterSpd) {
                if (random255() < 236)  // 236/256 ≈ 92%
                    score += 3;
            }
            if (hpPercent(caster) <= 70)
                score -= 1;
        }
        // ---------- Poison ----------
        else if (_move->ailment == Ailment::Poison || _move->ailment == Ailment::Toxic) {
            if (hpPercent(caster) < 50 || hpPercent(target) <= 50)
                score -= 1;
        }
        // ---------- Healing ----------
        else if (_move->healing > 0) {
            int userHp = hpPercent(caster);
            if (userHp == 100)
                score -= 3;
            if (effectiveSpeed(caster) > effectiveSpeed(target))
                score -= 8;
            if (userHp > 70) {
                if (random255() < 226)  // 226/256 ≈ 88%
                    score -= 3;
            } else {
                if (random255() < 236)  // 236/256 ≈ 92%
                    score += 2;
            }
        }
        // ---------- Stat boosts ----------
        else {
            // Attack boost
            if (_move->stat_changes[0] > 0) {
                int atkStage = caster.battleState.statModifiers[0];
                int userHp = hpPercent(caster);
                if (atkStage <= 2 && userHp == 100) {
                    if (random255() < 128)
                        score += 2;
                }
                if (atkStage >= 3) {
                    if (random255() < 156)
                        score -= 1;
                }
                if (userHp < 40)
                    score -= 2;
            }
            // Defense boost
            if (_move->stat_changes[1] > 0) {
                int defStage = caster.battleState.statModifiers[1];
                int userHp = hpPercent(caster);
                if (defStage <= 2 && userHp == 100) {
                    if (random255() < 128)
                        score += 2;
                }
                if (defStage >= 3) {
                    if (random255() < 156)
                        score -= 1;
                }
                if (userHp < 40)
                    score -= 2;
            }
            // Speed boost
            if (_move->stat_changes[4] > 0) {
                int casterSpd = effectiveSpeed(caster);
                int targetSpd = effectiveSpeed(target);
                if (casterSpd > targetSpd)
                    score -= 3;
                else
                    score += (random255() < 186 ? 3 : 0);
            }
            // Special Attack boost
            if (_move->stat_changes[2] > 0) {
                int spaStage = caster.battleState.statModifiers[2];
                int userHp = hpPercent(caster);
                if (spaStage <= 2 && userHp == 100) {
                    if (random255() < 128)
                        score += 2;
                }
                if (spaStage >= 3) {
                    if (random255() < 156)
                        score -= 1;
                }
                if (userHp < 40)
                    score -= 2;
            }
            // Special Defense boost
            if (_move->stat_changes[3] > 0) {
                int spdStage = caster.battleState.statModifiers[3];
                int userHp = hpPercent(caster);
                if (spdStage <= 2 && userHp == 100) {
                    if (random255() < 128)
                        score += 2;
                }
                if (spdStage >= 3) {
                    if (random255() < 156)
                        score -= 1;
                }
                if (userHp < 40)
                    score -= 2;
            }
            // Evasion boost
            if (_move->stat_changes[6] > 0) {
                int userHp = hpPercent(caster);
                if (userHp >= 90) {
                    if (random255() < 156)
                        score += 3;
                }
                int evaStage = caster.battleState.statModifiers[6];
                if (evaStage >= 3) {
                    if (random255() < 128)
                        score -= 1;
                }
                if (target.battleState.statusCondition == Ailment::Poison ||
                    target.battleState.statusCondition == Ailment::Toxic ||
                    target.battleState.statusCondition == Ailment::Burn) {
                    if (random255() < 186)
                        score += 3;
                }
            }
        }

        // ---------- Stat lowering moves ----------
        // Attack lowering
        if (_move->stat_changes[0] < 0) {
            if (target.battleState.statModifiers[0] != 0)
                score -= 1;
            if (hpPercent(caster) <= 90)
                score -= 1;
            if (target.battleState.statModifiers[0] <= -3) {
                if (random255() < 206)
                    score -= 2;
            }
            if (hpPercent(target) <= 70)
                score -= 2;
            else {
                if (random255() < 206)
                    score -= 2;
            }
        }
        // Defense lowering
        if (_move->stat_changes[1] < 0) {
            if (hpPercent(caster) < 70 || target.battleState.statModifiers[1] <= -3) {
                if (random255() < 206)
                    score -= 2;
            }
            if (hpPercent(caster) <= 70)
                score -= 2;
        }
        // Speed lowering
        if (_move->stat_changes[4] < 0) {
            int casterSpd = effectiveSpeed(caster);
            int targetSpd = effectiveSpeed(target);
            if (casterSpd > targetSpd)
                score -= 3;
            else
                score += (random255() < 186 ? 2 : 0);
        }
        // Special Attack lowering
        if (_move->stat_changes[2] < 0) {
            if (target.battleState.statModifiers[2] != 0)
                score -= 1;
            if (hpPercent(caster) <= 90)
                score -= 1;
            if (target.battleState.statModifiers[2] <= -3) {
                if (random255() < 206)
                    score -= 2;
            }
            if (hpPercent(target) <= 70)
                score -= 2;
            else {
                if (random255() < 206)
                    score -= 2;
            }
        }
        // Special Defense lowering
        if (_move->stat_changes[3] < 0) {
            if (hpPercent(caster) < 70 || target.battleState.statModifiers[3] <= -3) {
                if (random255() < 206)
                    score -= 2;
            }
            if (hpPercent(caster) <= 70)
                score -= 2;
        }
        // Accuracy lowering
        if (_move->stat_changes[5] < 0) {
            if (hpPercent(caster) < 70 || hpPercent(target) <= 70) {
                if (random255() < 156)
                    score -= 1;
            }
            if (target.battleState.statModifiers[5] <= -2) {
                if (random255() < 176)
                    score -= 2;
            }
            if (target.battleState.statusCondition == Ailment::Poison ||
                target.battleState.statusCondition == Ailment::Toxic ||
                target.battleState.statusCondition == Ailment::Burn){
                if (random255() < 186)
                    score += 2;
            }
        }
        // Evasion lowering
        if (_move->stat_changes[6] < 0) {
            if (hpPercent(caster) < 70 || target.battleState.statModifiers[6] <= -3) {
                if (random255() < 206)
                    score -= 2;
            }
            if (hpPercent(target) <= 70)
                score -= 2;
        }
    }

    return score;
}

int BattleAI::estimateDamage(const Move* _move, const Battler& caster, const Battler& target) const {
    if (_move->category == MoveCategory::NonDamaging) return 0;

    int attack = 0, defense = 0;
    if (_move->category == MoveCategory::PhysicalAtk) {
        attack = caster.pokeState.stats[1];
        defense = target.pokeState.stats[2];
        attack = PokeMath::applyStatModifier(attack, caster.battleState.statModifiers[0]);
        defense = PokeMath::applyStatModifier(defense, target.battleState.statModifiers[1]);
    } else { // SpecialAtk
        attack = caster.pokeState.stats[3];
        defense = target.pokeState.stats[4];
        attack = PokeMath::applyStatModifier(attack, caster.battleState.statModifiers[2]);
        defense = PokeMath::applyStatModifier(defense, target.battleState.statModifiers[3]);
    }

    int level = caster.pokeState.lvl;
    int power = _move->power;
    double stab = 1.0;
    for (int i = 0; i < 2; ++i) {
        if (caster.pokeState.types[i] && *caster.pokeState.types[i] == _move->type)
            stab = 1.5;
    }
    double effectiveness = getEffectiveness(_move, target);
    if (effectiveness == 0) return 0;

    // Use the average random factor (100) instead of random
    int damage = static_cast<int>((((2 * level / 5 + 2) * power * attack / defense) / 50 + 2) * stab * effectiveness);
    return damage;
}

bool BattleAI::canFaint(const Move* _move, const Battler& caster, const Battler& target) const {
    if (_move->category == MoveCategory::NonDamaging) return false;
    return estimateDamage(_move, caster, target) >= target.battleState.currentHealth;
}

double BattleAI::getEffectiveness(const Move* _move, const Battler& target) const {
    int eff = PokeTypes::getTypeEffectiveness(_move->type,
                target.pokeState.types[0] ? *target.pokeState.types[0] : Type::Null,
                target.pokeState.types[1] ? *target.pokeState.types[1] : Type::Null);
    return eff / 100.0;
}

int BattleAI::random255() const {
    std::uniform_int_distribution<int> dist(0, 255);
    return dist(m_rng);
}

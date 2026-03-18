#include <BattleMoveHandler.h>
#include "PokeMath/calculatePokeStats.h"
#include "data_move.h"
#include <cstring>
#include <globals.h>
#include <qdebug.h>
#include <PokeMath/processCatchAttempt.h>
#include <PokeMath/calculateDamage.h>
#include <PokemonTypes.h>


BattleMoveHandler::BattleMoveHandler(const PokemonState& wildState, const std::array<PokemonState, 6>& partyStates)
    : m_rng(std::random_device{}())
{
    m_battleOpponent = createBattler(wildState);
    for (int i = 0; i < 6; i++) {
        m_battleParty[i] = createBattler(partyStates[i]);
    }
}

Battler* BattleMoveHandler::createBattler(const PokemonState& state) {
    const Poke* poke = Globals::getPoke(state.pokedex_id);

    Battler* battler = new Battler();
    battler->pokeState.name = state.name;
    battler->pokeState.lvl = state.lvl;
    battler->pokeState.stats = calculatePokeStats(
        state.lvl,
        poke->base_stats,
        state.ivs,
        state.evs,
        PokemonTypes::getNatureMultipliers(state.nature)
    );
    battler->pokeState.types[0] = &poke->types[0];
    battler->pokeState.types[1] = &poke->types[1];

    for (int i = 0; i < 4; ++i) {
        battler->pokeState.moves[i] = Globals::getMove(state.moves[i]);
    }

    return battler;
}

void BattleMoveHandler::startActionRound(int actionIndex, QString _action){
    const char* action = _action.toStdString().data();
    qDebug() << "Starting action round:" << actionIndex << action;

    assert((!std::strcmp(action, "Switch") || !std::strcmp(action, "Fight") || !std::strcmp(action, "Catch"))
           && "Action must be 'Switch', 'Fight' or 'Catch'");
    assert(actionIndex>-1 && actionIndex<6 && "actionIndex must be between 0 and 5 inclusive");
    assert((!std::strcmp(action,"Switch") || actionIndex<4) && "actionIndex for non-switch action must be between 0 and 3 inclusive");

    int opponentMoveIndex = rand()%4;

    const Move* opponentMove = m_battleOpponent->pokeState.moves[opponentMoveIndex];
    const Move* playerMove = m_battleParty[m_chosenPartyIndex]->pokeState.moves[actionIndex];

    // Wipe battle state delta
    m_battleOpponent->delta = {};
    m_battleParty[m_chosenPartyIndex]->delta = {};

    m_battleOpponent->battleState.lastMoveIndex = opponentMoveIndex;
    m_battleParty[m_chosenPartyIndex]->battleState.lastMoveIndex = actionIndex;

    int switchedIn = -1;
    bool playerFirst = true;
    int shakes = -1;

    if(action[0]=='S'){
       m_chosenPartyIndex = actionIndex;
       switchedIn = actionIndex;
    } else if(action[0]=='C'){
        shakes = processCatchAttempt(m_rng, m_battleOpponent->pokeState.stats[0], m_battleOpponent->battleState.currentHealth, 50);
        if(shakes > 3){
            // Generate sequence for successful catch
            QVariantList sequence = generateActionSequence(*m_battleOpponent, *m_battleParty[m_chosenPartyIndex], playerFirst, switchedIn, shakes);
            qDebug() << "Catch success sequence generated with" << sequence.size() << "actions";
            emit actionSequenceReady(sequence);
            return;
        }
    }

    if(action[0]=='F'){
        if (playerMove->priority == opponentMove->priority){
           playerFirst = m_battleOpponent->pokeState.stats[5] < m_battleParty[m_chosenPartyIndex]->pokeState.stats[5];
        } else {
           playerFirst = opponentMove->priority < playerMove->priority;
        }

        if(playerFirst){
            // Player attacks first
            applyMove(playerMove, m_battleParty[m_chosenPartyIndex], m_battleOpponent);

            // Check if opponent can still attack (not fainted and not flinched)
            if(m_battleOpponent->battleState.currentHealth > 0 && !m_battleOpponent->delta.flinched) {
                applyMove(opponentMove, m_battleOpponent, m_battleParty[m_chosenPartyIndex]);
            }
        } else {
            // Opponent attacks first
            applyMove(opponentMove, m_battleOpponent, m_battleParty[m_chosenPartyIndex]);

            // Check if player can still attack (not fainted and not flinched)
            if(m_battleParty[m_chosenPartyIndex]->battleState.currentHealth > 0 && !m_battleParty[m_chosenPartyIndex]->delta.flinched) {
                applyMove(playerMove, m_battleParty[m_chosenPartyIndex], m_battleOpponent);
            }
        }
    } else {
        // For Switch or Catch actions, only opponent attacks
        assert(playerFirst && "Player should always go first if it isn't fighting");
        applyMove(opponentMove, m_battleOpponent, m_battleParty[m_chosenPartyIndex]);
    }

    qDebug() << "Battle round - playerFirst:" << playerFirst << "opponent damage:" << m_battleOpponent->delta.damage;

    // Generate the action sequence
    QVariantList sequence = generateActionSequence(*m_battleOpponent, *m_battleParty[m_chosenPartyIndex], playerFirst, switchedIn, shakes);
    qDebug() << "Action sequence generated with" << sequence.size() << "actions";
    emit actionSequenceReady(sequence);
}

bool BattleMoveHandler::canBattlerMove(Battler* caster) {
    // Check status conditions that prevent attacking
    if (caster->battleState.statusCondition == Ailment::Sleep) {
        caster->delta.sleep = true;
        return false;
    }

    if (caster->battleState.statusCondition == Ailment::Freeze) {
        caster->delta.freeze = true;
        return false;
    }

    if (caster->battleState.statusCondition == Ailment::Paralysis) {
        std::uniform_int_distribution<int> paraDist(1, 4);
        if (paraDist(m_rng) == 1) { // 25% chance to be paralyzed
            caster->delta.paralyzed = true;
            return false;
        }
    }

    // Check for confusion
    if (caster->battleState.confused == Ailment::Confusion) {
        std::uniform_int_distribution<int> confuseDist(1, 2);
        if (confuseDist(m_rng) == 1) { // 50% chance to hit self in confusion
            // Calculate confusion damage
            int confusionDamage = calculateConfusionDamage(caster->pokeState.lvl);
            caster->delta.confusedDamage = confusionDamage;
            return false; // Hurts self, doesn't attack opponent
        }
    }

    // Check for flinch (set by previous move)
    if (caster->delta.flinched) {
        return false;
    }

    return true;
}

int BattleMoveHandler::calculateConfusionDamage(int level) {
    // Confusion damage formula: typically (40 * level / 100) + 2
    return (40 * level / 100) + 2;
}

int BattleMoveHandler::calculateTypeEffectiveness(const Move* move, Battler* target) {
    Type targetTypes[2] = {target->pokeState.types[0] ? *target->pokeState.types[0] : Null,
                           target->pokeState.types[1] ? *target->pokeState.types[1] : Null};

    return PokemonTypes::getTypeEffectiveness(move->type, targetTypes);
}

void BattleMoveHandler::applyMove(const Move* move, Battler* caster, Battler* target){
    // First check if the battler can move at all
    if (!canBattlerMove(caster)) {
        return;
    }

    // Check accuracy (if move has less than 100% accuracy)
    if (move->accuracy < 100) {
        std::uniform_int_distribution<int> accuracyDist(1, 100);
        if (accuracyDist(m_rng) > move->accuracy) {
            caster->delta.miss = true;
            return;
        }
    }

    // Check for critical hit
    std::uniform_int_distribution<int> critDist(1, 16/(1+move->crit_rate));
    bool crit = critDist(m_rng) == 1;
    caster->delta.critical = crit;

    if(move->category != MoveCategory::NonDamaging){
        // Setup damage parameters
        DamageParams params;
        params.lvl = caster->pokeState.lvl;
        params.power = move->power;

        // Determine which attack/defense stats to use
        int attackStatIndex = 1; // Default to Attack for physical moves
        if(move->category == MoveCategory::SpecialAtk){
            attackStatIndex = 3; // Special Attack
        }

        params.attack = caster->pokeState.stats[attackStatIndex];
        params.defense = target->pokeState.stats[attackStatIndex + 2];

        // Apply burn penalty for physical attacks
        if(move->category == MoveCategory::PhysicalAtk){
            if(caster->battleState.statusCondition == Ailment::Burn) {
                params.burn = 50; // Burn reduces physical damage by 50%
            }
        }

        // Apply stat modifiers
        int atkModifier = caster->battleState.statModifiers[attackStatIndex];
        int defModifier = target->battleState.statModifiers[attackStatIndex + 2];

        // Critical hits ignore negative attack modifiers and positive defense modifiers
        if(crit){
            params.critical = 150; // Critical hit multiplier
            if (atkModifier < 0) atkModifier = 0;
            if (defModifier > 0) defModifier = 0;
        }

        params.attack = applyStatModifier(params.attack, atkModifier);
        params.defense = applyStatModifier(params.defense, defModifier);

        // Calculate STAB (Same Type Attack Bonus)
        bool hasStab = false;
        for (int i = 0; i < 2; ++i) {
            if (caster->pokeState.types[i] && *caster->pokeState.types[i] == move->type) {
                hasStab = true;
                break;
            }
        }
        params.stab = hasStab ? 150 : 100; // 1.5x for STAB

        // Calculate type1 and type2 effectiveness
        Type targetType1 = target->pokeState.types[0] ? *target->pokeState.types[0] : Null;
        Type targetType2 = target->pokeState.types[1] ? *target->pokeState.types[1] : Null;

        // Check for special moves: Struggle, Future Sight, Beat Up, Doom Desire
        // For now, assume all normal moves - you can add specific checks for these moves
        bool isSpecialMove = false; // Set to true for Struggle, Future Sight, Beat Up, Doom Desire

        if (!isSpecialMove) {
            // Calculate type1: effectiveness against first type
            Type targetTypes1[2] = {targetType1, Null};
            params.type1 = PokemonTypes::getTypeEffectiveness(move->type, targetTypes1);

            // Calculate type2: effectiveness against second type (100 if no second type)
            if (targetType2 != Null) {
                Type targetTypes2[2] = {targetType2, Null};
                params.type2 = PokemonTypes::getTypeEffectiveness(move->type, targetTypes2);
            } else {
                params.type2 = 100; // Default to 1x if no second type
            }
        } else {
            // For special moves, both type1 and type2 are always 100 (1x)
            params.type1 = 100;
            params.type2 = 100;
        }

        // Calculate combined effectiveness for super/not very effective messages
        int combinedEffectiveness = (params.type1 * params.type2) / 100;

        // Calculate actual damage
        int damage = calculateDamage(params, m_rng);
        caster->delta.damage = damage;

        // Apply damage to target
        target->battleState.currentHealth = std::max(0, target->battleState.currentHealth - damage);

        // Determine if super effective or not very effective
        if (combinedEffectiveness > 100) {
            caster->delta.superEffective = true;
        } else if (combinedEffectiveness < 100 && combinedEffectiveness > 0) {
            caster->delta.notVeryEffective = true;
        }

        // Handle drain moves (like Giga Drain, Leech Life)
        if (move->drain > 0) {
            int drainAmount = damage * move->drain / 100;
            caster->delta.drain = drainAmount;
            caster->battleState.currentHealth = std::min(100, caster->battleState.currentHealth + drainAmount);
        }

        // Handle healing moves
        if (move->healing > 0) {
            int healAmount = caster->pokeState.stats[0] * move->healing / 100; // Heal based on max HP
            caster->delta.heal = healAmount;
            caster->battleState.currentHealth = std::min(100, caster->battleState.currentHealth + healAmount);
        }

        // Check for secondary effects
        if (move->ailment_chance > 0) {
            std::uniform_int_distribution<int> ailmentDist(1, 100);
            if (ailmentDist(m_rng) <= move->ailment_chance) {
                caster->delta.addStatusCondition = move->ailment;
            }
        }

        if (move->flinch_chance > 0) {
            std::uniform_int_distribution<int> flinchDist(1, 100);
            if (flinchDist(m_rng) <= move->flinch_chance) {
                target->delta.flinched = true;
            }
        }

        // Handle stat changes
        if (move->stat_chance > 0) {
            std::uniform_int_distribution<int> statDist(1, 100);
            if (statDist(m_rng) <= move->stat_chance) {
                for (int i = 0; i < 5; ++i) {
                    if (move->stat_changes[i] != 0) {
                        caster->delta.deltaStatModifiers[i] = move->stat_changes[i];
                        // Apply to battle state
                        caster->battleState.statModifiers[i] = std::max(-6, std::min(6,
                            caster->battleState.statModifiers[i] + move->stat_changes[i]));
                    }
                }
            }
        }

    } else {
        // Non-damaging move (status move)
        // Handle status conditions
        if (move->ailment_chance > 0) {
            std::uniform_int_distribution<int> ailmentDist(1, 100);
            if (ailmentDist(m_rng) <= move->ailment_chance) {
                caster->delta.addStatusCondition = move->ailment;
                if (target->battleState.statusCondition == Ailment::Null) {
                    target->battleState.statusCondition = move->ailment;
                }
            }
        }

        // Handle stat changes for non-damaging moves
        if (move->stat_changes[0] != 0 || move->stat_changes[1] != 0 ||
            move->stat_changes[2] != 0 || move->stat_changes[3] != 0 ||
            move->stat_changes[4] != 0) {

            // Check if stat change succeeds
            bool success = true;
            if (move->stat_chance > 0 && move->stat_chance < 100) {
                std::uniform_int_distribution<int> statDist(1, 100);
                success = (statDist(m_rng) <= move->stat_chance);
            }

            if (success) {
                for (int i = 0; i < 5; ++i) {
                    if (move->stat_changes[i] != 0) {
                        // Determine if affecting self or target
                        // For simplicity, assume affecting target for now
                        target->delta.deltaStatModifiers[i] = move->stat_changes[i];
                        target->battleState.statModifiers[i] = std::max(-6, std::min(6,
                            target->battleState.statModifiers[i] + move->stat_changes[i]));
                    }
                }
            }
        }
    }
}
int BattleMoveHandler::applyStatModifier(int baseStat, int modifier) {
    if (modifier == 0) return baseStat;

    // Stat modifier multipliers: ±0:1, ±1:1.5, ±2:2, ±3:2.5, ±4:3, ±5:3.5, ±6:4
    static const float multiplier[13] = {
        0.25f, 0.28f, 0.33f, 0.40f, 0.50f, 0.66f, // -6 to -1
        1.0f,                                      // 0
        1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f        // +1 to +6
    };

    int index = modifier + 6; // Convert -6..+6 to 0..12
    return static_cast<int>(baseStat * multiplier[index]);
}

// Helper function to generate move sequence for a single battler
void BattleMoveHandler::generateMoveSequence(QVariantList& sequence, Battler& attacker, Battler& defender,
                                             const QString& attackerName, const QString& defenderName,
                                             const QString& attackerRole, const QString& defenderRole) {
    const Move* move = attacker.pokeState.moves[attacker.battleState.lastMoveIndex];
    QString moveName = QString::fromStdString(move->name);

    sequence.append(createTextAction(attackerName + " used " + moveName + "!", 300));

    if(attacker.delta.confusedDamage > 0) {
        sequence.append(createTextAction(attackerName + " hurt itself in its confusion!", 500));
        sequence.append(createDamageAction(attackerRole, attacker.delta.confusedDamage, 200));
        sequence.append(createHealthChangeAction(attackerRole, -attacker.delta.confusedDamage, 1000));
    } else if(attacker.delta.flinched) {
        sequence.append(createTextAction(attackerName + " flinched!", 1000));
    } else if(attacker.delta.sleep) {
        sequence.append(createTextAction(attackerName + " is fast asleep!", 1000));
    } else if(attacker.delta.freeze) {
        sequence.append(createTextAction(attackerName + " is frozen solid!", 1000));
    } else if(attacker.delta.paralyzed) {
        sequence.append(createTextAction(attackerName + " is paralyzed! It can't move!", 1000));
    } else if(attacker.delta.miss) {
        sequence.append(createAttackAction(attackerRole, 500));
        sequence.append(createTextAction(attackerName + "'s attack missed!", 1000));
    } else if(attacker.delta.damage > 0) {
        sequence.append(createAttackAction(attackerRole, 500));
        sequence.append(createDamageAction(defenderRole, attacker.delta.damage, 200));
        sequence.append(createHealthChangeAction(defenderRole, -defender.delta.damage, 1000));

        if(defender.delta.critical) {
            sequence.append(createTextAction("A critical hit!", 800));
        }
        if(defender.delta.superEffective) {
            sequence.append(createTextAction("It's super effective!", 800));
        } else if (defender.delta.notVeryEffective) {
            sequence.append(createTextAction("It's not very effective...", 800));
        }
        if(attacker.delta.drain > 0) {
            sequence.append(createTextAction(attackerName + " drained health!", 800));
            sequence.append(createHealthChangeAction(attackerRole, attacker.delta.drain, 800));
        }
    }

    addPostMoveEffects(sequence, attacker, attackerName, attackerRole == "player");
    addPostMoveEffects(sequence, defender, defenderName, defenderRole == "player");
}

QVariantList BattleMoveHandler::generateActionSequence(Battler& opponent, Battler& player, bool playerFirst, int switchedIn, int shakes){
    QVariantList sequence;

    // Debug print sequence parameters
    qDebug() << "Generating action sequence with: playerFirst =" << playerFirst
             << "switchedIn =" << switchedIn << "shakes =" << shakes;

    // Catch attempt
    if(shakes > -1) {
        if(shakes > 3) {
            // Successful catch
            sequence.append(createTextAction("Gotcha! " + QString::fromStdString(opponent.pokeState.name) + " was caught!", 2000));
            sequence.append(createCatchAction(shakes, true));
            return sequence;
        } else {
            // Failed catch - show escape and opponent attacks
            sequence.append(createTextAction("Aargh! Almost had it!", 300));
            sequence.append(createCatchAction(shakes, false));

            // Add opponent's move after failed catch
            generateMoveSequence(sequence, opponent, player, "Opponent", "Player", "opponent", "player");

            sequence.append(createEndAction());
            return sequence;
        }
    }

    // Switch scenario - only opponent attacks
    if(switchedIn > -1) {
        generateMoveSequence(sequence, opponent, player, "Opponent", "Player", "opponent", "player");
        sequence.append(createEndAction());
        return sequence;
    }

    // Regular battle turn
    QString firstAttackerName = playerFirst ? "Player" : "Opponent";
    QString firstDefenderName = playerFirst ? "Opponent" : "Player";
    QString firstAttackerRole = playerFirst ? "player" : "opponent";
    QString firstDefenderRole = playerFirst ? "opponent" : "player";

    QString secondAttackerName = playerFirst ? "Opponent" : "Player";
    QString secondDefenderName = playerFirst ? "Player" : "Opponent";
    QString secondAttackerRole = playerFirst ? "opponent" : "player";
    QString secondDefenderRole = playerFirst ? "player" : "opponent";

    // First attacker's turn
    Battler& firstAttacker = playerFirst ? player : opponent;
    Battler& firstDefender = playerFirst ? opponent : player;

    generateMoveSequence(sequence, firstAttacker, firstDefender,
                         firstAttackerName, firstDefenderName,
                         firstAttackerRole, firstDefenderRole);

    // Second attacker's turn (only if first defender didn't faint)
    if(firstDefender.battleState.currentHealth > 0) {
        Battler& secondAttacker = playerFirst ? opponent : player;
        Battler& secondDefender = playerFirst ? player : opponent;

        generateMoveSequence(sequence, secondAttacker, secondDefender,
                             secondAttackerName, secondDefenderName,
                             secondAttackerRole, secondDefenderRole);
    }

    sequence.append(createEndAction());

    // Debug print the entire sequence
    qDebug() << "=== Generated Action Sequence ===";
    for (int i = 0; i < sequence.size(); ++i) {
        QVariantMap action = sequence[i].toMap();
        qDebug() << "Action" << i << ":" << action;
    }
    qDebug() << "=== End Sequence ===";

    return sequence;
}

QString BattleMoveHandler::ailmentToString(Ailment ailment) {
    switch(ailment) {
        case Ailment::Burn: return "burn";
        case Ailment::Freeze: return "freeze";
        case Ailment::Paralysis: return "paralysis";
        case Ailment::Poison: return "poison";
        case Ailment::Sleep: return "sleep";
        case Ailment::Confusion: return "confusion";
        default: return "";
    }
}

void BattleMoveHandler::addPostMoveEffects(QVariantList& sequence, Battler& battler, const QString& name, bool isPlayer) {
    QString role = isPlayer ? "player" : "opponent";

    if(battler.delta.heal > 0) {
        sequence.append(createTextAction(name + " regained health!", 800));
        sequence.append(createHealthChangeAction(role, battler.delta.heal, 800));
    }

    if(battler.delta.addStatusCondition != Ailment::Null) {
        QString ailment = ailmentToString(battler.delta.addStatusCondition);
        sequence.append(createTextAction(name + " was " + ailment + "ed!", 800));
    }

    if(battler.delta.addConfusion) {
        sequence.append(createTextAction(name + " became confused!", 800));
    }

    if(battler.delta.ailmentDaamge > 0) {
        QString ailment = ailmentToString(battler.battleState.statusCondition);
        sequence.append(createTextAction(name + " is hurt by its " + ailment + "!", 500));
        sequence.append(createDamageAction(role, battler.delta.ailmentDaamge, 200));
        sequence.append(createHealthChangeAction(role, -battler.delta.ailmentDaamge, 800));
    }

    if(battler.delta.removeStatusCondition != Ailment::Null) {
        QString ailment = ailmentToString(battler.delta.removeStatusCondition);
        sequence.append(createTextAction(name + " is no longer " + ailment + "ed!", 800));
    }

    if(battler.delta.removeConfusion) {
        sequence.append(createTextAction(name + " snapped out of confusion!", 800));
    }

    // Add stat change messages
    for (int i = 0; i < 5; ++i) {
        if (battler.delta.deltaStatModifiers[i] > 0) {
            QString statName = getStatName(i);
            sequence.append(createTextAction(name + "'s " + statName + " rose!", 800));
        } else if (battler.delta.deltaStatModifiers[i] < 0) {
            QString statName = getStatName(i);
            sequence.append(createTextAction(name + "'s " + statName + " fell!", 800));
        }
    }
}

QString BattleMoveHandler::getStatName(int statIndex) {
    static const QString statNames[5] = {
        "Attack", "Defense", "Special Attack", "Special Defense", "Speed"
    };
    return (statIndex >= 0 && statIndex < 5) ? statNames[statIndex] : "Stat";
}

QVariantMap BattleMoveHandler::createTextAction(const QString& message, int delay) {
    QVariantMap action;
    action["type"] = "text";
    action["message"] = message;
    action["delay"] = delay;
    return action;
}

QVariantMap BattleMoveHandler::createAttackAction(const QString& role, int delay) {
    QVariantMap action;
    action["type"] = "attack";
    action["role"] = role;
    action["delay"] = delay;
    return action;
}

QVariantMap BattleMoveHandler::createDamageAction(const QString& role, int damage, int delay) {
    QVariantMap action;
    action["type"] = "damage";
    action["role"] = role;
    action["damage"] = damage;
    action["delay"] = delay;
    return action;
}

QVariantMap BattleMoveHandler::createHealthChangeAction(const QString& role, int amount, int delay) {
    QVariantMap action;
    action["type"] = "change-health";
    action["role"] = role;
    action["amount"] = amount;
    action["delay"] = delay;
    return action;
}

QVariantMap BattleMoveHandler::createCatchAction(int shakes, bool success) {
    QVariantMap action;
    action["type"] = "catch";
    action["shakes"] = shakes;
    action["success"] = success;
    return action;
}

QVariantMap BattleMoveHandler::createEndAction() {
    QVariantMap action;
    action["type"] = "end";
    return action;
}

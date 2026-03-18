#include <BattleMoveHandler.h>
#include "data_move.h"
#include <cstring>
#include <globals.h>
#include <qdebug.h>
#include <PokeMath.h>
#include <PokeTypes.h>
#include <random>

// BATTLE SYSTEM OVERVIEW
// ----------------------
// This system handles Pokémon battle mechanics with an event-driven architecture.
//
// HOW AN ACTION ROUND WORKS:
// 1. Player chooses action (Fight/Switch/Catch)
// 2. Opponent randomly selects a move (0-3)
// 3. Priority/speed determines who moves first
// 4. For Fight actions:
//    a. First battler attempts to move (applyMove)
//    b. If second battler is still alive and not flinched, they move
//    c. End-of-turn effects applied (burn/poison damage)
// 5. For Switch/Catch: opponent gets a free move

BattleMoveHandler::BattleMoveHandler(const PokemonState& wildState, const std::array<PokemonState, 6>& partyStates)
    : m_rng(std::random_device{}())
{
    m_battleOpponent = createBattler(wildState);
    for (int i = 0; i < 6; i++) {
        m_battleParty[i] = createBattler(partyStates[i]);
    }
}

BattleMoveHandler::~BattleMoveHandler() {
    delete m_battleOpponent;
    for (auto& ptr : m_battleParty) delete ptr;
}

Battler* BattleMoveHandler::createBattler(const PokemonState& state) {
    const Poke* poke = Globals::getPoke(state.pokedex_id);

    Battler* battler = new Battler();
    battler->pokeState.name = state.name;
    battler->pokeState.lvl = state.lvl;
    battler->pokeState.types[0] = &poke->types[0];
    battler->pokeState.types[1] = &poke->types[1];

    for (int i = 0; i < 4; i++) {
        battler->pokeState.moves[i] = Globals::getMove(state.moves[i]);
    }

    battler->pokeState.stats = PokeMath::calculatePokeStats(
        state.lvl,
        poke->base_stats,
        state.ivs,
        state.evs,
        PokeTypes::getNatureMultipliers(state.nature)
    );

    battler->battleState.currentHealth = battler->pokeState.stats[0];
    return battler;
}

void BattleMoveHandler::switchPartyMember(int newChosenIndex){
    Battler* currentMember = m_battleParty[m_chosenIndex];
    currentMember->battleState.statModifiers = {0, 0, 0, 0, 0};
    currentMember->battleState.confused = Ailment::Null;
    currentMember->battleState.confusedCounter = -1;
    m_chosenIndex = newChosenIndex;
}

void BattleMoveHandler::resetDeltaState(BattleStateDelta& delta) {
    delta = BattleStateDelta();
}

void BattleMoveHandler::startActionRound(int actionIndex, QString _action){
    std::string actionStr = _action.toStdString();
    const char* action = actionStr.data();

    assert((!std::strcmp(action, "Switch") || !std::strcmp(action, "Fight") || !std::strcmp(action, "Catch"))
           && "Action must be 'Switch', 'Fight' or 'Catch'");
    assert(actionIndex>-1 && actionIndex<6 && "actionIndex must be between 0 and 5 inclusive");
    assert((!std::strcmp(action,"Switch") || actionIndex<4) && "actionIndex for non-switch action must be between 0 and 3 inclusive");

    resetDeltaState(m_battleOpponent->delta);
    resetDeltaState(m_battleParty[m_chosenIndex]->delta);

    std::uniform_int_distribution<int> moveChoiceDist(0, 3);
    int opponentMoveIndex = moveChoiceDist(m_rng);

    const Move* opponentMove = m_battleOpponent->pokeState.moves[opponentMoveIndex];
    Battler* player = m_battleParty[m_chosenIndex];
    const Move* playerMove = player->pokeState.moves[actionIndex];

    m_battleOpponent->battleState.lastMoveIndex = opponentMoveIndex;
    player->battleState.lastMoveIndex = actionIndex;

    int switchedIn = -1;
    bool playerFirst = true;
    int shakes = -1;

    if(action[0]=='S'){
       switchedIn = actionIndex;
    } else if(action[0]=='C'){
        shakes = PokeMath::calculateBallShakes(m_rng, m_battleOpponent->pokeState.stats[0], m_battleOpponent->battleState.currentHealth, 50);
        m_battleOpponent->delta.flinched = false;
        player->delta.flinched = false;

        QVariantList s;
        s.append(createTextAction("Player used one Poké Ball!", 300));
        s.append(createCatchAction(shakes, ms_catchStart));
        s.append(createEndAction());
        emit actionSequenceReady(s);
        return;
    }

    if(action[0]=='F'){
        if (playerMove->priority == opponentMove->priority){
            int oppSpeed = m_battleOpponent->pokeState.stats[5];
            if(m_battleOpponent->battleState.statusCondition==Ailment::Paralysis) oppSpeed = oppSpeed/2;
            int oppModifier = m_battleOpponent->battleState.statModifiers[4];
            oppSpeed = applyStatModifier(oppSpeed, oppModifier);

            int playerSpeed = player->pokeState.stats[5];
            if(player->battleState.statusCondition==Ailment::Paralysis) playerSpeed = playerSpeed/2;
            int playerModifier = player->battleState.statModifiers[4];
            playerSpeed = applyStatModifier(playerSpeed, playerModifier);

           if(playerSpeed == oppSpeed){
               std::uniform_int_distribution<int> speedTieDist(0, 1);
               playerFirst = speedTieDist(m_rng)==0;
           }else{
               playerFirst = playerSpeed > oppSpeed;
           }
        } else {
           playerFirst = opponentMove->priority < playerMove->priority;
        }

        m_battleOpponent->delta.isFirst = !playerFirst;
        player->delta.isFirst = playerFirst;

        BattleActionResult turnResult;

        if(playerFirst){
            BattleActionResult playerResult = applyMove(playerMove, player, m_battleOpponent);
            applyBattleResult(playerResult);
            turnResult.effects.insert(turnResult.effects.end(), playerResult.effects.begin(), playerResult.effects.end());

            if(m_battleOpponent->battleState.currentHealth > 0 && !m_battleOpponent->delta.flinched) {
                BattleActionResult opponentResult = applyMove(opponentMove, m_battleOpponent, player);
                applyBattleResult(opponentResult);
                turnResult.effects.insert(turnResult.effects.end(), opponentResult.effects.begin(), opponentResult.effects.end());
            }
        } else {
            BattleActionResult opponentResult = applyMove(opponentMove, m_battleOpponent, player);
            applyBattleResult(opponentResult);
            turnResult.effects.insert(turnResult.effects.end(), opponentResult.effects.begin(), opponentResult.effects.end());

            if(player->battleState.currentHealth > 0 && !player->delta.flinched) {
                BattleActionResult playerResult = applyMove(playerMove, player, m_battleOpponent);
                applyBattleResult(playerResult);
                turnResult.effects.insert(turnResult.effects.end(), playerResult.effects.begin(), playerResult.effects.end());
            }
        }

        BattleActionResult playerEndResult = applyEndOfTurnEffects(player);
        applyBattleResult(playerEndResult);
        turnResult.effects.insert(turnResult.effects.end(), playerEndResult.effects.begin(), playerEndResult.effects.end());

        BattleActionResult opponentEndResult = applyEndOfTurnEffects(m_battleOpponent);
        applyBattleResult(opponentEndResult);
        turnResult.effects.insert(turnResult.effects.end(), opponentEndResult.effects.begin(), opponentEndResult.effects.end());

        QVariantList s = generateSequenceFromResult(turnResult);
        s.append(createEndAction());

        logActionSequence(s);
        emit actionSequenceReady(s);

    } else {
        player->delta.flinched = false;
        m_battleOpponent->delta.flinched = false;

        BattleActionResult switchResult;
        if(switchedIn > -1) {
            BattleActionResult opponentResult = applyMove(opponentMove, m_battleOpponent, player);
            applyBattleResult(opponentResult);
            switchResult.effects.insert(switchResult.effects.end(), opponentResult.effects.begin(), opponentResult.effects.end());
        }

        QVariantList s = generateSequenceFromResult(switchResult);
        s.append(createEndAction());

        logActionSequence(s);
        emit actionSequenceReady(s);
    }
}

void BattleMoveHandler::checkRemoveAilment(Battler& battler, BattleActionResult& result){
    if(battler.battleState.confused==Ailment::Confusion){
        if(battler.battleState.confusedCounter >= battler.battleState.confusedTurns){
            result.addEffect(BattleActionResult::CONFUSION_REMOVED, &battler, &battler);
            battler.battleState.confused = Ailment::Null;
            battler.battleState.confusedCounter = -1;
            battler.battleState.confusedTurns = -1;
        }
    }

    if(battler.battleState.statusCondition!=Ailment::Null){
        if(battler.battleState.conditionTurns > 0 && battler.battleState.conditionCounter >= battler.battleState.conditionTurns){
            result.addEffect(BattleActionResult::STATUS_REMOVED, &battler, &battler, 0, battler.battleState.statusCondition);
            battler.battleState.statusCondition = Ailment::Null;
            battler.battleState.conditionCounter = -1;
            battler.battleState.conditionTurns = -1;
        }
    }
}

BattleActionResult BattleMoveHandler::canBattlerMove(Battler* caster) {
    BattleActionResult result;

    if (caster->battleState.statusCondition == Ailment::Sleep) {
        result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                        caster->pokeState.name + " is fast asleep!");
        result.moveExecuted = false;
        return result;
    }

    if (caster->battleState.statusCondition == Ailment::Freeze) {
        if (PokeMath::calculateFreezeThaw(m_rng)) {
            result.addEffect(BattleActionResult::STATUS_REMOVED, caster, caster, 0, Ailment::Freeze);
            result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                            caster->pokeState.name + " thawed out!");
        } else {
            result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                            caster->pokeState.name + " is frozen solid!");
            result.moveExecuted = false;
            return result;
        }
    }

    if (caster->battleState.statusCondition == Ailment::Paralysis) {
        if (!PokeMath::calculateParalysisHit(m_rng)) {
            result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                            caster->pokeState.name + " is paralyzed! It can't move!");
            result.moveExecuted = false;
            return result;
        }
    }

    if (caster->battleState.confused == Ailment::Confusion) {
        std::uniform_int_distribution<int> confuseDist(1, 2);
        if (confuseDist(m_rng) == 1) {
            PokeMath::DamageParams confP;
            confP.lvl = caster->pokeState.lvl;
            confP.power = 40;

            int atkModifier = caster->battleState.statModifiers[0];
            int defModifier = caster->battleState.statModifiers[1];
            confP.attack = caster->pokeState.stats[1];
            confP.defense = caster->pokeState.stats[2];
            confP.attack = applyStatModifier(confP.attack, atkModifier);
            confP.defense = applyStatModifier(confP.defense, defModifier);

            int confusionDamage = PokeMath::calculateDamage(confP, m_rng);
            result.addEffect(BattleActionResult::CONFUSION_SELF_HIT, caster, caster, confusionDamage);
            result.moveExecuted = false;
            return result;
        }
    }

    if (caster->delta.flinched) {
        result.addEffect(BattleActionResult::FLINCH, caster, nullptr);
        result.moveExecuted = false;
        return result;
    }

    result.moveExecuted = true;
    return result;
}

BattleActionResult BattleMoveHandler::applyEndOfTurnEffects(Battler* battler) {
    BattleActionResult result;

    if (battler->battleState.statusCondition == Ailment::Burn) {
        int burnDamage = PokeMath::calculateBurnDamage(battler->pokeState.stats[0]);
        result.addEffect(BattleActionResult::DAMAGE, battler, battler, burnDamage);
        result.addEffect(BattleActionResult::TEXT, battler, nullptr, 0, Ailment::Null, -1, 0,
                        battler->pokeState.name + " is hurt by its burn!");
    }

    if (battler->battleState.statusCondition == Ailment::Poison || battler->battleState.statusCondition == Ailment::Toxic) {
        int counter = -1;
        if(battler->battleState.statusCondition == Ailment::Toxic) {
            counter = battler->battleState.conditionCounter;
        }
        int poisonDamage = PokeMath::calculatePoisonDamage(battler->pokeState.stats[0], counter);
        result.addEffect(BattleActionResult::DAMAGE, battler, battler, poisonDamage);
        std::string ailment = battler->battleState.statusCondition == Ailment::Toxic ? "bad poison" : "poison";
        result.addEffect(BattleActionResult::TEXT, battler, nullptr, 0, Ailment::Null, -1, 0,
                        battler->pokeState.name + " is hurt by its " + ailment + "!");
    }

    return result;
}

BattleActionResult BattleMoveHandler::applyMove(const Move* _move, Battler* caster, Battler* target) {
    BattleActionResult result;
    checkRemoveAilment(*caster, result);

    BattleActionResult canMoveResult = canBattlerMove(caster);
    if (!canMoveResult.moveExecuted) {
        result.effects.insert(result.effects.end(), canMoveResult.effects.begin(), canMoveResult.effects.end());
        return result;
    }

    result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                    caster->pokeState.name + " used " + std::string(_move->name) + "!");

    if (_move->accuracy < 100) {
        std::uniform_int_distribution<int> accuracyDist(1, 100);
        if (accuracyDist(m_rng) > _move->accuracy) {
            result.addEffect(BattleActionResult::MISS, caster, target);
            result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                            caster->pokeState.name + "'s attack missed!");
            return result;
        }
    }

    if(_move->category != MoveCategory::NonDamaging){
        PokeMath::DamageParams params;
        params.lvl = caster->pokeState.lvl;
        params.power = _move->power;

        std::uniform_int_distribution<int> critDist(1, 16/(1+_move->crit_rate));
        bool crit = critDist(m_rng) == 1;
        if (crit) {
            result.addEffect(BattleActionResult::CRITICAL, caster, target);
        }

        int attackStatIndex = 1;
        if(_move->category == MoveCategory::SpecialAtk){
            attackStatIndex = 3;
        }

        int atkModifier = caster->battleState.statModifiers[attackStatIndex - 1];
        int defModifier = target->battleState.statModifiers[attackStatIndex];

        params.attack = caster->pokeState.stats[attackStatIndex];
        params.defense = target->pokeState.stats[attackStatIndex + 1];

        if(crit){
            params.critical = 150;
            if (atkModifier < 0) atkModifier = 0;
            if (defModifier > 0) defModifier = 0;
        }

        params.attack = applyStatModifier(params.attack, atkModifier);
        params.defense = applyStatModifier(params.defense, defModifier);

        if(_move->category == MoveCategory::PhysicalAtk){
            if(caster->battleState.statusCondition == Ailment::Burn) params.burn = 50;
        }

        bool hasStab = false;
        for (int i = 0; i < 2; i++) {
            if (caster->pokeState.types[i] && *caster->pokeState.types[i] == _move->type) {
                hasStab = true;
                break;
            }
        }
        params.stab = hasStab ? 150 : 100;

        const Type* targetType1 = target->pokeState.types[0];
        const Type* targetType2 = target->pokeState.types[1];

        params.type1 = PokeTypes::getTypeEffectiveness(_move->type, *targetType1);

        if (*targetType2 != Type::Null) {
            params.type2 = PokeTypes::getTypeEffectiveness(_move->type, *targetType2);
        } else {
            params.type2 = 100;
        }

        int damage = PokeMath::calculateDamage(params, m_rng);
        result.addEffect(BattleActionResult::DAMAGE, caster, target, damage);

        int combinedEffectiveness = params.type1 * params.type2;
        if (combinedEffectiveness > 10000) {
            result.addEffect(BattleActionResult::SUPER_EFFECTIVE, caster, target);
        } else if (combinedEffectiveness<10000 && combinedEffectiveness > 0) {
            result.addEffect(BattleActionResult::NOT_VERY_EFFECTIVE, caster, target);
        } else if (combinedEffectiveness==0){
            result.addEffect(BattleActionResult::NO_EFFECT, caster, target);
        }

        if (_move->drain > 0) {
            int drainAmount = damage * _move->drain / 100;
            result.addEffect(BattleActionResult::DRAIN, caster, target, drainAmount);
        }

        if (_move->healing > 0) {
            int healAmount = caster->pokeState.stats[0] * _move->healing / 100;
            result.addEffect(BattleActionResult::HEAL, caster, caster, healAmount);
        }

        BattleActionResult secondaryResult = applySecondaryEffects(_move, target, damage > 0);
        result.effects.insert(result.effects.end(), secondaryResult.effects.begin(), secondaryResult.effects.end());

    } else {
        BattleActionResult secondaryResult = applySecondaryEffects(_move, target, true);
        result.effects.insert(result.effects.end(), secondaryResult.effects.begin(), secondaryResult.effects.end());
    }

    return result;
}

BattleActionResult BattleMoveHandler::applySecondaryEffects(const Move* _move, Battler* target, bool damageLanded) {
    BattleActionResult result;

    bool ailmentApplied = true;
    bool statApplied = true;

    if(_move->category != MoveCategory::NonDamaging){
        std::uniform_int_distribution<int> effectDist(1, 100);
        ailmentApplied = effectDist(m_rng) <= _move->ailment_chance;
        statApplied = effectDist(m_rng) <= _move->stat_chance;
    }

    if (ailmentApplied && _move->ailment != Ailment::Null) {
        if(_move->ailment == Ailment::Confusion){
            if(target->battleState.confused == Ailment::Confusion) {
                result.addEffect(BattleActionResult::TEXT, nullptr, target, 0, Ailment::Null, -1, 0, "But it failed!");
            } else {
                result.addEffect(BattleActionResult::CONFUSION_ADDED, nullptr, target);
                target->battleState.confused = Ailment::Confusion;
                target->battleState.confusedTurns = PokeMath::calculateAilmentTurns(Ailment::Confusion, m_rng);
                target->battleState.confusedCounter = 0;
            }
        } else {
            if(target->battleState.statusCondition != Ailment::Null) {
                if(_move->category == MoveCategory::NonDamaging) {
                    result.addEffect(BattleActionResult::TEXT, nullptr, target, 0, Ailment::Null, -1, 0, "But it failed!");
                }
            } else {
                result.addEffect(BattleActionResult::STATUS_APPLIED, nullptr, target, 0, _move->ailment);
                target->battleState.statusCondition = _move->ailment;
                target->battleState.conditionCounter = 0;
                target->battleState.conditionTurns = PokeMath::calculateAilmentTurns(_move->ailment, m_rng);
            }
        }
    }

    if (damageLanded && _move->flinch_chance > 0) {
        std::uniform_int_distribution<int> flinchDist(1, 100);
        if (flinchDist(m_rng) <= _move->flinch_chance) {
            result.addEffect(BattleActionResult::FLINCH, nullptr, target);
            target->delta.flinched = true;
        }
    }

    if (statApplied) {
        for (int i = 0; i < 5; i++) {
            if (_move->stat_changes[i] != 0) {
                int currentModifier = target->battleState.statModifiers[i];
                int newModifier = currentModifier + _move->stat_changes[i];
                int clampedModifier = std::max(-6, std::min(6, newModifier));

                int actualChange = clampedModifier - currentModifier;

                if(actualChange == 0) {
                    if(_move->stat_changes[i] > 0) {
                        result.addEffect(BattleActionResult::TEXT, nullptr, target, 0, Ailment::Null, -1, 0,
                                        target->pokeState.name + "'s " + getStatName(i).toStdString() + " won't go any higher!");
                    } else {
                        result.addEffect(BattleActionResult::TEXT, nullptr, target, 0, Ailment::Null, -1, 0,
                                        target->pokeState.name + "'s " + getStatName(i).toStdString() + " won't go any lower!");
                    }
                } else {
                    result.addEffect(BattleActionResult::STAT_CHANGED, nullptr, target, 0, Ailment::Null, i, actualChange);
                    target->battleState.statModifiers[i] = clampedModifier;
                }
            }
        }
    }

    return result;
}

void BattleMoveHandler::applyBattleResult(const BattleActionResult& result) {
    for (const auto& effect : result.effects) {
        switch(effect.type) {
            case BattleActionResult::DAMAGE:
                if (effect.target && effect.amount > 0) {
                    effect.target->battleState.currentHealth =
                        std::max(0, effect.target->battleState.currentHealth - effect.amount);
                }
                break;

            case BattleActionResult::HEAL:
                if (effect.target && effect.amount > 0) {
                    effect.target->battleState.currentHealth =
                        std::min(effect.target->pokeState.stats[0],
                                effect.target->battleState.currentHealth + effect.amount);
                }
                break;

            case BattleActionResult::DRAIN:
                if (effect.source && effect.amount > 0) {
                    effect.source->battleState.currentHealth =
                        std::min(effect.source->pokeState.stats[0],
                                effect.source->battleState.currentHealth + effect.amount);
                }
                break;

            case BattleActionResult::CONFUSION_SELF_HIT:
                if (effect.target && effect.amount > 0) {
                    effect.target->battleState.currentHealth =
                        std::max(0, effect.target->battleState.currentHealth - effect.amount);
                }
                break;

            case BattleActionResult::STATUS_REMOVED:
                if (effect.target) {
                    effect.target->battleState.statusCondition = Ailment::Null;
                    effect.target->battleState.conditionCounter = -1;
                    effect.target->battleState.conditionTurns = -1;
                }
                break;

            case BattleActionResult::CONFUSION_REMOVED:
                if (effect.target) {
                    effect.target->battleState.confused = Ailment::Null;
                    effect.target->battleState.confusedCounter = -1;
                    effect.target->battleState.confusedTurns = -1;
                }
                break;

            default:
                break;
        }
    }
}

QVariantList BattleMoveHandler::generateSequenceFromResult(const BattleActionResult& result) {
    QVariantList sequence;

    for (const auto& effect : result.effects) {
        QString sourceName = effect.source ? QString::fromStdString(effect.source->pokeState.name) : "";
        QString targetName = effect.target ? QString::fromStdString(effect.target->pokeState.name) : "";
        QString sourceRole = effect.source ? (effect.source == m_battleOpponent ? "opponent" : "player") : "";
        QString targetRole = effect.target ? (effect.target == m_battleOpponent ? "opponent" : "player") : "";

        switch(effect.type) {
            case BattleActionResult::TEXT:
                sequence.append(createTextAction(QString::fromStdString(effect.text), ms_moveUsedText));
                break;

            case BattleActionResult::DAMAGE:
                if (effect.amount > 0) {
                    if (!sourceRole.isEmpty()) {
                        sequence.append(createAttackAction(sourceRole, ms_attackAnimation));
                    }
                    if (!targetRole.isEmpty()) {
                        sequence.append(createDamageAction(targetRole, ms_damageAnimation));
                        sequence.append(createHealthChangeAction(targetRole, -effect.amount, ms_healthChange));
                    }
                }
                break;

            case BattleActionResult::HEAL:
                if (effect.amount > 0 && !targetRole.isEmpty()) {
                    sequence.append(createHealthChangeAction(targetRole, effect.amount, ms_healthChange));
                }
                break;

            case BattleActionResult::DRAIN:
                if (!sourceRole.isEmpty() && effect.amount > 0) {
                    sequence.append(createTextAction(sourceName + " drained health!", ms_drainEffectText));
                    sequence.append(createHealthChangeAction(sourceRole, effect.amount, ms_healthChange));
                }
                break;

            case BattleActionResult::CRITICAL:
                sequence.append(createTextAction("A critical hit!", ms_criticalHitText));
                break;

            case BattleActionResult::SUPER_EFFECTIVE:
                sequence.append(createTextAction("It's super effective!", ms_effectivenessText));
                break;

            case BattleActionResult::NOT_VERY_EFFECTIVE:
                sequence.append(createTextAction("It's not very effective...", ms_effectivenessText));
                break;

            case BattleActionResult::NO_EFFECT:
                sequence.append(createTextAction("It doesn't affect " + targetName + "...", ms_effectivenessText));
                break;

            case BattleActionResult::MISS:
                sequence.append(createTextAction(sourceName + "'s attack missed!", ms_statusConditionText));
                break;

            case BattleActionResult::FLINCH:
                sequence.append(createTextAction(targetName + " flinched!", ms_statusConditionText));
                break;

            case BattleActionResult::STATUS_APPLIED:
                if (effect.target) {
                    sequence.append(createStatusCondition(targetRole, effect.ailment));
                    QString ailmentText = ailmentToApplicationText(effect.ailment);
                    sequence.append(createTextAction(targetName + " " + ailmentText, ms_statusConditionText));
                }
                break;

            case BattleActionResult::STATUS_REMOVED:
                if (effect.target) {
                    sequence.append(createStatusCondition(targetRole, Ailment::Null));
                    QString ailment = ailmentToRemovalText(effect.ailment);
                    sequence.append(createTextAction(targetName + " is no longer " + ailment + "!", ms_statusConditionText));
                }
                break;

            case BattleActionResult::CONFUSION_ADDED:
                sequence.append(createTextAction(targetName + " became confused!", ms_statusConditionText));
                break;

            case BattleActionResult::CONFUSION_REMOVED:
                sequence.append(createTextAction(targetName + " snapped out of confusion!", ms_statusConditionText));
                break;

            case BattleActionResult::CONFUSION_SELF_HIT:
                if (effect.amount > 0) {
                    sequence.append(createTextAction("It hurt itself in its confusion!", ms_ailmentText));
                    if (!targetRole.isEmpty()) {
                        sequence.append(createDamageAction(targetRole, ms_damageAnimation));
                        sequence.append(createHealthChangeAction(targetRole, -effect.amount, ms_healthChange));
                    }
                } else {
                    sequence.append(createTextAction("It hurt itself in its confusion!", ms_ailmentText));
                    sequence.append(createTextAction("But it had no effect!", ms_statusConditionText));
                }
                break;

            case BattleActionResult::STAT_CHANGED:
                if (effect.statIndex >= 0 && effect.statIndex < 5) {
                    QString statName = getStatName(effect.statIndex);
                    if(effect.statChange == 1) {
                        sequence.append(createTextAction(targetName + "'s " + statName + " rose!", ms_statusConditionText));
                    } else if(effect.statChange >= 2) {
                        sequence.append(createTextAction(targetName + "'s " + statName + " rose sharply!", ms_statusConditionText));
                    } else if(effect.statChange == -1) {
                        sequence.append(createTextAction(targetName + "'s " + statName + " fell!", ms_statusConditionText));
                    } else if(effect.statChange <= -2) {
                        sequence.append(createTextAction(targetName + "'s " + statName + " harshly fell!", ms_statusConditionText));
                    }
                }
                break;
        }
    }

    return sequence;
}

int BattleMoveHandler::applyStatModifier(int baseStat, int modifier) {
    if (modifier == 0) return baseStat;

    modifier = std::max(-6, std::min(6, modifier));

    static const float multiplier[13] = {
        0.25f, 0.28f, 0.33f, 0.40f, 0.50f, 0.66f,
        1.0f,
        1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f
    };

    int index = modifier + 6;
    return static_cast<int>(baseStat * multiplier[index]);
}

QVariantList BattleMoveHandler::generateActionSequence(Battler& opponent, Battler& player, bool playerFirst, int switchedIn, int shakes){
    QVariantList s;
    return s;
}

void BattleMoveHandler::logActionSequence(const QVariantList& s) {
    qDebug() << "========================================";
    qDebug() << "       ACTION SEQUENCE LOG";
    qDebug() << "========================================";
    qDebug() << "Total actions:" << s.size();

    for (int i = 0; i < s.size(); i++) {
        QVariantMap action = s[i].toMap();
        QString type = action["type"].toString();

        qDebug() << "[" << i << "]" << type.toUpper();

        if (type == "text") {
            qDebug() << "    Message:" << action["message"].toString();
            qDebug() << "    Delay:  " << action["delay"].toInt() << "ms";
        }
        else if (type == "attack") {
            qDebug() << "    Role:   " << action["role"].toString();
            qDebug() << "    Delay:  " << action["delay"].toInt() << "ms";
        }
        else if (type == "damage") {
            qDebug() << "    Role:   " << action["role"].toString();
            qDebug() << "    Delay:  " << action["delay"].toInt() << "ms";
        }
        else if (type == "change-health") {
            qDebug() << "    Role:   " << action["role"].toString();
            qDebug() << "    Amount: " << action["amount"].toInt();
            qDebug() << "    Delay:  " << action["delay"].toInt() << "ms";
        }
        else if (type == "attempt-catch") {
            qDebug() << "    Shakes: " << action["shakes"].toInt();
        }
        else if (type == "end") {
            qDebug() << "    (End of sequence)";
        }
        else if (type == "status-condition") {
            qDebug() << "    Role:   " << action["role"].toString();
            qDebug() << "    Label:  " << action["label"].toString();
            qDebug() << "    Remove: " << action["remove"].toBool();
        }
    }

    qDebug() << "";
}

QString BattleMoveHandler::ailmentToApplicationText(Ailment ailment){
    switch(ailment) {
        case Ailment::Burn: return "was burned!";
        case Ailment::Freeze: return "was frozen solid!";
        case Ailment::Paralysis: return "is paralyzed!";
        case Ailment::Poison: return "was poisoned!";
        case Ailment::Toxic: return "was badly poisoned!";
        case Ailment::Sleep: return "fell asleep!";
        case Ailment::Confusion: return "became confused!";
        default: return "";
    }
};

QString BattleMoveHandler::ailmentToHurtText(Ailment ailment){
    switch(ailment) {
        case Ailment::Burn: return "burn";
        case Ailment::Poison: return "poison";
        case Ailment::Toxic: return "poison";
        default: return "";
    }
};

QString BattleMoveHandler::ailmentToRemovalText(Ailment ailment){
    switch(ailment) {
        case Ailment::Burn: return "burn";
        case Ailment::Freeze: return "freeze";
        case Ailment::Paralysis: return "paralysis";
        case Ailment::Poison: return "poison";
        case Ailment::Toxic: return "poison";
        case Ailment::Sleep: return "sleep";
        default: return "";
    }
};

QString BattleMoveHandler::getStatName(int statIndex) {
    static const QString statNames[5] = {
        "Attack", "Defense", "Special Attack", "Special Defense", "Speed"
    };
    return (statIndex >= 0 && statIndex < 5) ? statNames[statIndex] : "Stat";
}

QVariantMap BattleMoveHandler::createStatusCondition(const QString& role, Ailment ailment){
    QVariantMap action;
    action["type"] = "status-condition";
    action["role"] = role;
    action["remove"] = false;
    switch(ailment){
        case Ailment::Burn: action["label"] = "BRN"; break;
        case Ailment::Freeze: action["label"] = "FRZ"; break;
        case Ailment::Paralysis: action["label"] = "PAR"; break;
        case Ailment::Sleep: action["label"] = "SLP"; break;
        case Ailment::Poison: action["label"] = "PSN"; break;
        case Ailment::Toxic: action["label"] = "PSN"; break;
        case Ailment::Null: action["remove"] = true; break;
        default: action["label"] = ""; break;
    }
    return action;
};

QVariantMap BattleMoveHandler::createEndAction() {
    QVariantMap action;
    action["type"] = "end";
    return action;
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

QVariantMap BattleMoveHandler::createDamageAction(const QString& role, int delay) {
    QVariantMap action;
    action["type"] = "damage";
    action["role"] = role;
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

QVariantMap BattleMoveHandler::createCatchAction(int shakes, int delay) {
    QVariantMap action;
    action["type"] = "attempt-catch";
    action["shakes"] = shakes;
    action["delay"] = delay;
    return action;
}

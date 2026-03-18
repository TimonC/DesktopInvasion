#include <BattleMoveHandler.h>
#include "data_move.h"
#include <cstring>
#include <globals.h>
#include <qdebug.h>
#include <PokeMath.h>
#include <PokeTypes.h>
#include <random>

BattleMoveHandler::BattleMoveHandler(const PokemonState& wildState, const std::array<PokemonState, 6>& partyStates)
    : m_rng(std::random_device{}())
{
    qDebug() << "BattleMoveHandler constructor called!";
    m_battleOpponent = createBattler(wildState);
    for (int i = 0; i < 6; i++) {
        m_battleParty[i] = createBattler(partyStates[i]);
    }
    m_includedPartyIndices.push_back(m_chosenIndex);
}

BattleMoveHandler::~BattleMoveHandler() {
    qDebug() << "BattleMoveHandler destructor called!";
    delete m_battleOpponent;
    for (auto& ptr : m_battleParty) delete ptr;
}

Battler* BattleMoveHandler::createBattler(const PokemonState& state) {
    const Poke* poke = Globals::getPoke(state.pokedex_id);

    Battler* battler = new Battler();
    battler->pokeState.name = state.name;
    battler->pokeState.lvl = state.lvl;
    battler->pokeState.catchRate = poke->catch_rate;
    battler->pokeState.baseXP= poke->base_xp;
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

std::array<int, 6> BattleMoveHandler::getExperienceSpread(){
    std::array<int,6> spread = {-1,-1,-1,-1,-1,-1};
    int includedCount = 0;

    for(int index = 0; index < m_includedPartyIndices.size(); index++){
        int partyIndex = m_includedPartyIndices[index];
        Battler* member = m_battleParty[partyIndex];
        if(member->battleState.currentHealth >= 0 && member->pokeState.lvl<100){
            includedCount++;
        }
    }

    int xp = PokeMath::calculateExperience(m_battleOpponent->pokeState.lvl, includedCount, m_battleOpponent->pokeState.baseXP);
    for(int index = 0; index < m_includedPartyIndices.size(); index++){
        int partyIndex = m_includedPartyIndices[index];
        Battler* member = m_battleParty[partyIndex];
        if(member->battleState.currentHealth > 0 && member->pokeState.lvl<100){
            spread[partyIndex] = xp;
        }
    }

    return spread;
}

QString BattleMoveHandler::switchPartyMember(int newChosenIndex){
    Battler* currentMember = m_battleParty[m_chosenIndex];
    currentMember->battleState.statModifiers = {0, 0, 0, 0, 0};
    currentMember->battleState.confused = Ailment::Null;
    currentMember->battleState.confusedCounter = -1;

    m_chosenIndex = newChosenIndex;
    m_includedPartyIndices.push_back(m_chosenIndex);

    Battler* newMember = m_battleParty[m_chosenIndex];
    return ailmentToLabel(newMember->battleState.statusCondition);
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

    static std::uniform_int_distribution<int> moveChoiceDist(0, 3);
    int opponentMoveIndex = moveChoiceDist(m_rng);

    const Move* opponentMove = m_battleOpponent->pokeState.moves[opponentMoveIndex];
    Battler* player = m_battleParty[m_chosenIndex];
    const Move* playerMove = player->pokeState.moves[actionIndex];

    m_battleOpponent->battleState.lastMoveIndex = opponentMoveIndex;
    player->battleState.lastMoveIndex = actionIndex;

    int switchedIn = -1;
    bool playerFirst = true;
    int shakes = -1;

    QVariantList s;

    if(action[0]=='S'){
       switchedIn = actionIndex;
       m_chosenIndex = actionIndex;
    } else if(action[0]=='C'){
        shakes = PokeMath::calculateBallShakes(m_rng, m_battleOpponent->pokeState.stats[0], m_battleOpponent->battleState.currentHealth, m_battleOpponent->pokeState.catchRate);
        m_battleOpponent->delta.flinched = false;
        player->delta.flinched = false;

        s.append(createTextAction("Player used one Poké Ball!", ms_ballUsed));
        s.append(createCatchAction(shakes, ms_catchStart));
    }

    if(action[0]=='F'){
        if (playerMove->priority == opponentMove->priority){
            int oppSpeed = m_battleOpponent->pokeState.stats[5];
            if(m_battleOpponent->battleState.statusCondition==Ailment::Paralysis) oppSpeed = oppSpeed/2;
            int oppModifier = m_battleOpponent->battleState.statModifiers[4];
            oppSpeed = PokeMath::applyStatModifier(oppSpeed, oppModifier);

            int playerSpeed = player->pokeState.stats[5];
            if(player->battleState.statusCondition==Ailment::Paralysis) playerSpeed = playerSpeed/2;
            int playerModifier = player->battleState.statModifiers[4];
            playerSpeed = PokeMath::applyStatModifier(playerSpeed, playerModifier);

           if(playerSpeed == oppSpeed){
               playerFirst = PokeMath::checkSpeedTie(m_rng);
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

        if(player->battleState.confused == Ailment::Confusion) {
            player->battleState.confusedCounter++;
        }
        if(m_battleOpponent->battleState.confused == Ailment::Confusion) {
            m_battleOpponent->battleState.confusedCounter++;
        }
        if(player->battleState.statusCondition != Ailment::Null) {
            player->battleState.conditionCounter++;
        }
        if(m_battleOpponent->battleState.statusCondition != Ailment::Null) {
            m_battleOpponent->battleState.conditionCounter++;
        }

        BattleActionResult playerEndResult = applyEndOfTurnEffects(player);
        applyBattleResult(playerEndResult);
        turnResult.effects.insert(turnResult.effects.end(), playerEndResult.effects.begin(), playerEndResult.effects.end());

        BattleActionResult opponentEndResult = applyEndOfTurnEffects(m_battleOpponent);
        applyBattleResult(opponentEndResult);
        turnResult.effects.insert(turnResult.effects.end(), opponentEndResult.effects.begin(), opponentEndResult.effects.end());

        s = generateSequenceFromResult(turnResult);
        s.append(createEndAction());

        logActionSequence(s);
        emit actionSequenceReady(s);

    } else {
        // For Switch or Catch actions, player action happens first
        player->delta.flinched = false;
        m_battleOpponent->delta.flinched = false;

        BattleActionResult opponentResult = applyMove(opponentMove, m_battleOpponent, player);
        applyBattleResult(opponentResult);

        if(action[0]=='C' && shakes == 4){
            s.append(createEndAction());
        } else {
            QVariantList attackSequence = generateSequenceFromResult(opponentResult);
            s = s + attackSequence;
            s.append(createEndAction());
        }

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
        if(battler.battleState.statusCondition == Ailment::Sleep){
            if(battler.battleState.conditionTurns > 0 && battler.battleState.conditionCounter >= battler.battleState.conditionTurns){
                result.addEffect(BattleActionResult::STATUS_REMOVED, &battler, &battler, 0, battler.battleState.statusCondition);
                battler.battleState.statusCondition = Ailment::Null;
                battler.battleState.conditionCounter = -1;
                battler.battleState.conditionTurns = -1;
            }
        }
        else if(battler.battleState.statusCondition == Ailment::Freeze){
            if(PokeMath::calculateFreezeBreak(m_rng)){
                result.addEffect(BattleActionResult::STATUS_REMOVED, &battler, &battler, 0, battler.battleState.statusCondition);
                battler.battleState.statusCondition = Ailment::Null;
                battler.battleState.conditionCounter = -1;
                battler.battleState.conditionTurns = -1;
            }
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
        result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                        caster->pokeState.name + " is frozen solid!");
        result.moveExecuted = false;
        return result;
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
        result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                        caster->pokeState.name + " is confused!");

        if (PokeMath::calculateConfusionHit(m_rng)) {
            PokeMath::DamageParams confP;
            confP.lvl = caster->pokeState.lvl;
            confP.power = 40;

            int atkModifier = caster->battleState.statModifiers[0];
            int defModifier = caster->battleState.statModifiers[1];
            confP.attack = caster->pokeState.stats[1];
            confP.defense = caster->pokeState.stats[2];
            confP.attack = PokeMath::applyStatModifier(confP.attack, atkModifier);
            confP.defense = PokeMath::applyStatModifier(confP.defense, defModifier);

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
    result.effects.insert(result.effects.end(), canMoveResult.effects.begin(), canMoveResult.effects.end());

    if (!canMoveResult.moveExecuted) {
        return result;
    }
    result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                    caster->pokeState.name + " used " + std::string(_move->name) + "!");

    if (_move->accuracy < 100) {
        if (!PokeMath::checkAccuracy(_move->accuracy, m_rng)) {
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

        if (PokeMath::checkCriticalHit(_move->crit_rate, m_rng)) {
            result.addEffect(BattleActionResult::CRITICAL, caster, target);
            params.critical = 150;
        }

        int attackStatIndex = 1;
        if(_move->category == MoveCategory::SpecialAtk){
            attackStatIndex = 3;
        }

        int atkModifier = caster->battleState.statModifiers[attackStatIndex - 1];
        int defModifier = target->battleState.statModifiers[attackStatIndex];

        params.attack = caster->pokeState.stats[attackStatIndex];
        params.defense = target->pokeState.stats[attackStatIndex + 1];

        if(params.critical == 150){
            if (atkModifier < 0) atkModifier = 0;
            if (defModifier > 0) defModifier = 0;
        }

        params.attack = PokeMath::applyStatModifier(params.attack, atkModifier);
        params.defense = PokeMath::applyStatModifier(params.defense, defModifier);

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
        ailmentApplied = PokeMath::checkSecondaryEffect(_move->ailment_chance, m_rng);
        statApplied = PokeMath::checkSecondaryEffect(_move->stat_chance, m_rng);
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
                if(_move->ailment == Ailment::Sleep){
                    target->battleState.conditionTurns = PokeMath::calculateAilmentTurns(_move->ailment, m_rng);
                } else {
                    target->battleState.conditionTurns = -1;
                }
            }
        }
    }

    if (damageLanded && _move->flinch_chance > 0) {
        if (PokeMath::checkSecondaryEffect(_move->flinch_chance, m_rng)) {
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
                    sequence.append(createStatusCondition(targetRole, effect.ailment, false));
                    QString ailmentText = ailmentToApplicationText(effect.ailment);
                    sequence.append(createTextAction(targetName + " " + ailmentText, ms_statusConditionText));
                }
                break;

            case BattleActionResult::STATUS_REMOVED:
                if (effect.target) {
                    sequence.append(createStatusCondition(targetRole, Ailment::Null, true));
                    QString ailment = ailmentToRemovalText(effect.ailment, targetName);
                    sequence.append(createTextAction(ailment, ms_statusConditionText));
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

QString BattleMoveHandler::ailmentToRemovalText(Ailment ailment, const QString& pokemonName){
    switch(ailment) {
        case Ailment::Burn: return pokemonName + " is no longer burned!";
        case Ailment::Freeze: return pokemonName + " thawed out!";
        case Ailment::Paralysis: return pokemonName + " is no longer paralyzed!";
        case Ailment::Poison: return pokemonName + " is no longer poisoned!";
        case Ailment::Toxic: return pokemonName + " is no longer poisoned!";
        case Ailment::Sleep: return pokemonName + " woke up!";
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

QString BattleMoveHandler::getStatName(int statIndex) {
    static const QString statNames[5] = {
        "Attack", "Defense", "Special Attack", "Special Defense", "Speed"
    };
    return (statIndex >= 0 && statIndex < 5) ? statNames[statIndex] : "Stat";
}

const QString BattleMoveHandler::ailmentToLabel(Ailment ailment){
    switch(ailment){
        case Ailment::Burn: return "BRN";
        case Ailment::Freeze: return "FRZ";
        case Ailment::Paralysis: return "PAR";
        case Ailment::Sleep: return "SLP";
        case Ailment::Poison: return "PSN";
        case Ailment::Toxic: return "PSN";
        case Ailment::Null: return "";
        default: return "";
    }
}
QVariantMap BattleMoveHandler::createStatusCondition(const QString& role, Ailment ailment, bool remove){
    QVariantMap action;
    action["type"] = "status-condition";
    action["role"] = role;
    action["remove"] = remove;
    action["label"] = ailmentToLabel(ailment);
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

    // Store the appropriate text based on shakes
    if (shakes == 0) {
        action["message"] = "Oh no! The Pokémon broke free!";
    } else if (shakes < 4) {
        action["message"] = "Aww! It appeared to be caught!";
    } else {
        action["message"] = "Gotcha! " + QString::fromStdString(m_battleOpponent->pokeState.name) + " was caught!";
    }

    return action;
}

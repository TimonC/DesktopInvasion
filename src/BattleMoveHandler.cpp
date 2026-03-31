#include <BattleMoveHandler.h>
#include "data_move.h"
#include <cstring>
#include <globals.h>
#include <qdebug.h>
#include <PokeMath.h>
#include <PokeTypes.h>
#include <random>
#include <lookup.h>

BattleMoveHandler::BattleMoveHandler(const PokemonState& wildState, const std::array<PokemonState, 6>& partyStates, std::string playerName, std::mt19937 &rng)
    : m_rng(rng)
    , m_expShare(Globals::expShare())
    , m_playerName(QString::fromStdString(playerName))
    , m_battleAI(rng)
{
    qDebug() << "BattleMoveHandler constructor called!";

    m_battleOpponent = createBattler(wildState);
    bool end = false;
    int n_moves = 0;
    for(auto _move : m_battleOpponent->pokeState.moves){
        if(_move==nullptr) break;
        n_moves+=1;
    };
    if(n_moves==0){//fallback for pokemon without moves
        m_battleOpponent->pokeState.moves[0] = Lookup::getMove(1);
        n_moves+=1;;
    }
    m_moveChoiceDist = std::uniform_int_distribution<int>(0, n_moves-1);

    for (int i = 0; i < 6; i++) {
        if(partyStates[i].pokedex_id<=0 || partyStates[i].pokedex_id >493){
        //band-aid solution, apparantly my entire code for some reason depends upon empty party slots being filled with an arbitrary pokemon
        //in the past, my database  would  fill empty party slots with the wild pokemon
        //I am too lazy to debug this for now and it doesn't really cause issues since the empty party slots are inaccessible
        //But ideally, empty party slots should just be able to be nullptrs without silently crashing everything
            m_battleParty[i] = createBattler(wildState);
        }else{
            m_battleParty[i] = createBattler(partyStates[i]);
            m_nrInParty+=1;
        }
    }
    m_includedPartyIndices.insert(m_chosenIndex);
}

BattleMoveHandler::~BattleMoveHandler() {
    qDebug() << "BattleMoveHandler destructor called!";
    delete m_battleOpponent;
    for (auto& ptr : m_battleParty) {
        if (ptr) {
            delete ptr;
            ptr = nullptr;
        }
    }
}

Battler* BattleMoveHandler::createBattler(const PokemonState& state) {
    const Poke* poke = Lookup::getPoke(state.pokedex_id);

    Battler* battler = new Battler();
    battler->pokeState.name = state.name;
    battler->pokeState.lvl = state.lvl;
    battler->pokeState.catchRate = poke->catch_rate;
    battler->pokeState.baseXP= poke->base_xp;
    battler->pokeState.types[0] = &poke->types[0];
    battler->pokeState.types[1] = &poke->types[1];

    for (int i = 0; i < 4; i++) {
        battler->pokeState.moves[i] = Lookup::getMove(state.moves[i]);
    }

    battler->pokeState.stats = PokeMath::calculatePokeStats(
        state.lvl,
        poke->base_stats,
        PokeTypes::getNatureMultipliers(state.nature)
    );

    battler->battleState.currentHealth = battler->pokeState.stats[0];
    return battler;
}

std::array<int, 6> BattleMoveHandler::getExperienceSpread(){
    std::array<int,6> spread = {-1,-1,-1,-1,-1,-1};
    if(!m_expShare){
        int includedCount = 0;
        for(int partyIndex : m_includedPartyIndices){
            Battler* member = m_battleParty[partyIndex];
            if(member->battleState.currentHealth >= 0 && member->pokeState.lvl<100){
                includedCount++;
            }
        }
        int xp = PokeMath::calculateExperience(m_battleOpponent->pokeState.lvl, includedCount, m_battleOpponent->pokeState.baseXP);
        for(int partyIndex : m_includedPartyIndices){
            Battler* member = m_battleParty[partyIndex];
            if(member->battleState.currentHealth > 0 && member->pokeState.lvl<100){
                spread[partyIndex] = xp;
            }
        }
    }else{
        int xp = PokeMath::calculateExperience(m_battleOpponent->pokeState.lvl, m_nrInParty, m_battleOpponent->pokeState.baseXP);
        for(int partyIndex = 0; partyIndex<m_nrInParty; partyIndex++){
            Battler* member = m_battleParty[partyIndex];
            if(member->battleState.currentHealth > 0 && member->pokeState.lvl<100){
                spread[partyIndex] = xp;
            }
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
    m_includedPartyIndices.insert(m_chosenIndex);

    Battler* newMember = m_battleParty[m_chosenIndex];
    return ailmentToLabel(newMember->battleState.statusCondition);
}

void BattleMoveHandler::resetDeltaState(BattleStateDelta& delta) {
    delta = BattleStateDelta();
}

void BattleMoveHandler::incrementConditionCounters() {
    Battler* player = m_battleParty[m_chosenIndex];

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
}

QVariantList BattleMoveHandler::processEndOfTurnEffects() {
    QVariantList endSequence;

    BattleActionResult playerEndResult = applyEndOfTurnEffects(m_battleParty[m_chosenIndex]);
    applyBattleResult(playerEndResult);
    QVariantList playerEndSequence = generateSequenceFromResult(playerEndResult);
    endSequence.append(playerEndSequence);

    BattleActionResult opponentEndResult = applyEndOfTurnEffects(m_battleOpponent);
    applyBattleResult(opponentEndResult);
    QVariantList opponentEndSequence = generateSequenceFromResult(opponentEndResult);
    endSequence.append(opponentEndSequence);

    return endSequence;
}

void BattleMoveHandler::startActionRound(int actionIndex, QString _action){
    QChar actionChar = _action.isEmpty() ? QChar() : _action[0];

    if (actionChar != 'S' && actionChar != 'F' && actionChar != 'C') {
        qFatal("Action must be 'Switch', 'Fight' or 'Catch'");
    }

    if(actionIndex < 0 || actionIndex >= 6) {
        qFatal("actionIndex must be between 0 and 5 inclusive");
    }

    if(actionChar != 'S' && actionIndex >= 4) {
        qFatal("actionIndex for non-switch action must be between 0 and 3 inclusive");
    }

    resetDeltaState(m_battleOpponent->delta);
    resetDeltaState(m_battleParty[m_chosenIndex]->delta);

    int opponentMoveIndex = m_battleAI.selectMove(*m_battleOpponent, *m_battleParty[m_chosenIndex]);

    const Move* opponentMove = m_battleOpponent->pokeState.moves[opponentMoveIndex];
    Battler* player = m_battleParty[m_chosenIndex];
    const Move* playerMove = player->pokeState.moves[actionIndex];

    m_battleOpponent->battleState.lastMoveIndex = opponentMoveIndex;
    player->battleState.lastMoveIndex = actionIndex;

    int switchedIn = -1;
    bool playerFirst = true;
    int shakes = -1;

    QVariantList s;

    if(actionChar == 'S'){
       switchedIn = actionIndex;
       m_chosenIndex = actionIndex;

    } else if(actionChar == 'C'){
        int catchMod;
        switch(actionIndex) {
            case 0:  // Poké Ball
                catchMod = 100;
                break;
            case 1:  // Great Ball
                catchMod = 150;
                break;
            case 2:  // Ultra Ball
                catchMod = 200;
                break;
            case 3:  // Master Ball
                catchMod = 255;
                break;
            default:
                catchMod = 100;
                break;
        }

        int statusMod;
        switch(m_battleOpponent->battleState.statusCondition) {
            case Ailment::Freeze:
            case Ailment::Sleep:
                statusMod = 250;
                break;
            case Ailment::Paralysis:
            case Ailment::Burn:
            case Ailment::Poison:
            case Ailment::Toxic:
                statusMod = 150;
                break;
            case Ailment::Null:
            case Ailment::Confusion:
            default:
                statusMod = 100;
                break;
        }

        shakes = PokeMath::calculateBallShakes(
            m_rng,
            m_battleOpponent->pokeState.stats[0],
            m_battleOpponent->battleState.currentHealth,
            m_battleOpponent->pokeState.catchRate,
            catchMod,
            statusMod
        );

        QString ballMessage = "";
        if(actionIndex==0){
            ballMessage = m_playerName + QStringLiteral(" used one Poké Ball!");
        }else if(actionIndex==1){
            ballMessage = m_playerName + QStringLiteral(" used one Great Ball!");
        }else if(actionIndex==2){
            ballMessage = m_playerName + QStringLiteral(" used one Ultra Ball!");
        }else{
            ballMessage = m_playerName + QStringLiteral(" used one Master Ball!");
        }
        s.append(createTextAction(ballMessage, ms_ballUsed));
        s.append(createCatchAction(shakes, ms_catchStart, actionIndex));
    }
    if(actionChar == 'F'){
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
        turnResult.effects.reserve(20);

        if(playerFirst){
            BattleActionResult playerResult = applyMove(playerMove, player, m_battleOpponent);
            applyBattleResult(playerResult);
            turnResult.effects.reserve(turnResult.effects.size() + playerResult.effects.size());
            turnResult.effects.insert(turnResult.effects.end(), playerResult.effects.begin(), playerResult.effects.end());

            if(m_battleOpponent->battleState.currentHealth > 0) {
                if (!m_battleOpponent->delta.flinched) {
                    BattleActionResult opponentResult = applyMove(opponentMove, m_battleOpponent, player, true);
                    applyBattleResult(opponentResult);
                    turnResult.effects.reserve(turnResult.effects.size() + opponentResult.effects.size());
                    turnResult.effects.insert(turnResult.effects.end(), opponentResult.effects.begin(), opponentResult.effects.end());
                } else {
                    BattleActionResult flinchResult;
                    flinchResult.addEffect(BattleActionResult::FLINCH, m_battleOpponent, nullptr);
                    turnResult.effects.reserve(turnResult.effects.size() + flinchResult.effects.size());
                    turnResult.effects.insert(turnResult.effects.end(), flinchResult.effects.begin(), flinchResult.effects.end());
                }
            }
        } else {
            BattleActionResult opponentResult = applyMove(opponentMove, m_battleOpponent, player);
            applyBattleResult(opponentResult);
            turnResult.effects.reserve(turnResult.effects.size() + opponentResult.effects.size());
            turnResult.effects.insert(turnResult.effects.end(), opponentResult.effects.begin(), opponentResult.effects.end());

            if(player->battleState.currentHealth > 0) {
                if (!player->delta.flinched) {
                    BattleActionResult playerResult = applyMove(playerMove, player, m_battleOpponent, true);
                    applyBattleResult(playerResult);
                    turnResult.effects.reserve(turnResult.effects.size() + playerResult.effects.size());
                    turnResult.effects.insert(turnResult.effects.end(), playerResult.effects.begin(), playerResult.effects.end());
                } else {
                    BattleActionResult flinchResult;
                    flinchResult.addEffect(BattleActionResult::FLINCH, player, nullptr);
                    turnResult.effects.reserve(turnResult.effects.size() + flinchResult.effects.size());
                    turnResult.effects.insert(turnResult.effects.end(), flinchResult.effects.begin(), flinchResult.effects.end());
                }
            }
        }

        incrementConditionCounters();

        QVariantList endSequence = processEndOfTurnEffects();
        s = generateSequenceFromResult(turnResult);
        s.append(endSequence);
        s.append(createEndAction());

        emit actionSequenceReady(s);

    } else {
        player->delta.flinched = false;
        m_battleOpponent->delta.flinched = false;

        BattleActionResult opponentResult = applyMove(opponentMove, m_battleOpponent, player);
        applyBattleResult(opponentResult);

        incrementConditionCounters();

        QVariantList endSequence = processEndOfTurnEffects();

        if(actionChar == 'C' && shakes == 4){
            s.append(createEndAction());
        } else {
            QVariantList attackSequence = generateSequenceFromResult(opponentResult);
            s.reserve(s.size() + attackSequence.size() + endSequence.size() + 1);
            s.append(attackSequence);
            s.append(endSequence);
            s.append(createEndAction());
        }

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
                        (caster->pokeState.name + " is fast asleep!").c_str());
        result.moveExecuted = false;
        return result;
    }

    if (caster->battleState.statusCondition == Ailment::Freeze) {
        result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                        (caster->pokeState.name + " is frozen solid!").c_str());
        result.moveExecuted = false;
        return result;
    }

    if (caster->battleState.statusCondition == Ailment::Paralysis) {
        if (!PokeMath::calculateParalysisHit(m_rng)) {
            result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                            (caster->pokeState.name + " is paralyzed! It can't move!").c_str());
            result.moveExecuted = false;
            return result;
        }
    }

    if (caster->battleState.confused == Ailment::Confusion) {
        result.addEffect(BattleActionResult::CONFUSED_TURN, caster, nullptr, 0, Ailment::Null, -1, 0,
                        (caster->pokeState.name + " is confused!").c_str());

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

    switch(battler->battleState.statusCondition) {
        case Ailment::Burn: {
            result.addEffect(BattleActionResult::TEXT, battler, nullptr, 0, Ailment::Null, -1, 0,
                            (battler->pokeState.name + " is hurt by its burn!").c_str());

            int burnDamage = PokeMath::calculateBurnDamage(battler->pokeState.stats[0]);
            result.addEffect(BattleActionResult::CHANGE_HEALTH_END_OF_TURN, nullptr, battler, burnDamage);
            break;
        }
        case Ailment::Poison: {
            result.addEffect(BattleActionResult::TEXT, battler, nullptr, 0, Ailment::Null, -1, 0, (battler->pokeState.name + " is hurt by poison!").c_str());
            int poisonDamage = PokeMath::calculatePoisonDamage(battler->pokeState.stats[0], -1);
            result.addEffect(BattleActionResult::CHANGE_HEALTH_END_OF_TURN, nullptr, battler, poisonDamage);
            break;
        }
        case Ailment::Toxic: {
            result.addEffect(BattleActionResult::TEXT, battler, nullptr, 0, Ailment::Null, -1, 0, (battler->pokeState.name + " is badly hurt by poison!").c_str());
            int poisonDamage = PokeMath::calculatePoisonDamage(battler->pokeState.stats[0], battler->battleState.conditionCounter);
            result.addEffect(BattleActionResult::CHANGE_HEALTH_END_OF_TURN, nullptr, battler, poisonDamage);
            break;
        }
        default:
            break;
    }

    return result;
}

BattleActionResult BattleMoveHandler::applyMove(const Move* _move, Battler* caster, Battler* target, bool otherHasHadTurn) {
    BattleActionResult result;
    checkRemoveAilment(*caster, result);

    BattleActionResult canMoveResult = canBattlerMove(caster);
    result.effects.reserve(result.effects.size() + canMoveResult.effects.size());
    result.effects.insert(result.effects.end(), canMoveResult.effects.begin(), canMoveResult.effects.end());

    if (!canMoveResult.moveExecuted) {
        return result;
    }
    std::string moveName = _move->name;
    moveName.erase(std::remove(moveName.begin(), moveName.end(), '-'), moveName.end());
    result.addEffect(BattleActionResult::TEXT, caster, nullptr, 0, Ailment::Null, -1, 0,
                    (caster->pokeState.name + " used " + moveName + "!").c_str());

    int accModifier = caster->battleState.statModifiers[5] + target->battleState.statModifiers[6];
    accModifier = std::min(std::max(accModifier,-6),6);
    if (_move->accuracy > 0 && _move->category != MoveCategory::NonDamaging && !PokeMath::checkAccuracy(_move->accuracy, accModifier, m_rng)) {
        result.addEffect(BattleActionResult::MISS, caster, target);
        return result;
    }

    if(_move->id==150){ //Splash exception!
        result.addEffect(BattleActionResult::SPLASH, caster, target);
        return result;
    }


    const Type* targetType1 = target->pokeState.types[0];
    const Type* targetType2 = target->pokeState.types[1];
    int typeEffectiveness1 = PokeTypes::getTypeEffectiveness(_move->type, *targetType1);
    int typeEffectiveness2  = 100;
    if (*targetType2 != Type::Null) typeEffectiveness2 = PokeTypes::getTypeEffectiveness(_move->type, *targetType2);
    int combinedEffectiveness = typeEffectiveness1 * typeEffectiveness2;

    bool damageLanded = true;
    if(_move->category != MoveCategory::NonDamaging){
        PokeMath::DamageParams params;
        params.lvl = caster->pokeState.lvl;
        params.power = _move->power;

        bool isCritical = PokeMath::checkCriticalHit(_move->crit_rate, m_rng);
        if (isCritical) params.critical = 150;

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

        params.type1 = typeEffectiveness1;
        params.type2 = typeEffectiveness2;

        int damage = PokeMath::calculateDamage(params, m_rng);

        result.addEffect(BattleActionResult::CHANGE_HEALTH, caster, target, damage);

        if (combinedEffectiveness==0){
            result.addEffect(BattleActionResult::NO_EFFECT, caster, target);
        }else{
            if(isCritical) result.addEffect(BattleActionResult::CRITICAL, caster, target);
            if (combinedEffectiveness > 10000) {
                result.addEffect(BattleActionResult::SUPER_EFFECTIVE, caster, target);
            } else if (combinedEffectiveness<10000 && combinedEffectiveness > 0) {
                result.addEffect(BattleActionResult::NOT_VERY_EFFECTIVE, caster, target);
            }
            if (_move->drain > 0) {
                int drainAmount = PokeMath::calculateDrain(damage, _move->drain);
                result.addEffect(BattleActionResult::DRAIN, caster, target, drainAmount);
            } else if (_move->drain < 0) {
                int recoilAmount = PokeMath::calculateDrain(damage, -_move->drain);
                result.addEffect(BattleActionResult::RECOIL, caster, caster, recoilAmount);
            }
        }

        if (_move->healing > 0) {
            int healAmount = PokeMath::calculateHeal(caster->pokeState.stats[0], _move->healing);
            result.addEffect(BattleActionResult::HEAL, caster, caster, healAmount);
        }

        damageLanded  = damage > 0;
    } else {
        if (_move->healing > 0) {
            int healAmount = PokeMath::calculateHeal(caster->pokeState.stats[0], _move->healing);
            result.addEffect(BattleActionResult::HEAL, caster, caster, healAmount);
        }
    }

    if (combinedEffectiveness==0 && (_move->ailment!=Ailment::Confusion)){
        result.addEffect(BattleActionResult::NO_EFFECT, caster, target);
    }else{
        BattleActionResult secondaryResult = applySecondaryEffects(_move, caster, target, true, otherHasHadTurn);
        result.effects.reserve(result.effects.size() + secondaryResult.effects.size());
        result.effects.insert(result.effects.end(), secondaryResult.effects.begin(), secondaryResult.effects.end());
    }
    return result;
}

BattleActionResult BattleMoveHandler::applySecondaryEffects(const Move* _move, Battler* caster, Battler* target, bool damageLanded, bool otherHasHadTurn) {
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
                result.addEffect(BattleActionResult::TEXT, nullptr, target, 0, Ailment::Null, -1, 0, QStringLiteral("But it failed!").toStdString());
            } else {
                result.addEffect(BattleActionResult::CONFUSION_ADDED, nullptr, target);
                target->battleState.confused = Ailment::Confusion;
                target->battleState.confusedTurns = PokeMath::calculateAilmentTurns(Ailment::Confusion, m_rng);
                target->battleState.confusedCounter = 0;
            }
        } else {
            if(target->battleState.statusCondition != Ailment::Null) {
                if(_move->category == MoveCategory::NonDamaging) {
                    result.addEffect(BattleActionResult::TEXT, nullptr, target, 0, Ailment::Null, -1, 0, QStringLiteral("But it failed!").toStdString());
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

    if (damageLanded && _move->flinch_chance > 0 && !otherHasHadTurn) {
        if (PokeMath::checkSecondaryEffect(_move->flinch_chance, m_rng)) {
            target->delta.flinched = true;
        }
    }

    if (statApplied) {
        Battler& statGetter = _move->stat_change_target==0 ? *caster : *target;
        for (int i = 0; i < 7; i++) {
            if (_move->stat_changes[i] != 0) {
                int currentModifier = statGetter.battleState.statModifiers[i];
                int newModifier = currentModifier + _move->stat_changes[i];
                int clampedModifier = std::max(-6, std::min(6, newModifier));

                int actualChange = clampedModifier - currentModifier;

                if(actualChange == 0) {
                    if(_move->stat_changes[i] > 0) {
                        result.addEffect(BattleActionResult::TEXT, nullptr, &statGetter, 0, Ailment::Null, -1, 0,
                                        (statGetter.pokeState.name + "'s " + getStatName(i).toStdString() + " won't go any higher!").c_str());
                    } else {
                        result.addEffect(BattleActionResult::TEXT, nullptr, &statGetter, 0, Ailment::Null, -1, 0,
                                        (statGetter.pokeState.name + "'s " + getStatName(i).toStdString() + " won't go any lower!").c_str());
                    }
                } else {
                    result.addEffect(BattleActionResult::STAT_CHANGED, nullptr, &statGetter, 0, Ailment::Null, i, actualChange);
                    statGetter.battleState.statModifiers[i] = clampedModifier;
                }
            }
        }
    }

    return result;
}

void BattleMoveHandler::applyBattleResult(const BattleActionResult& result) {
    for (const auto& effect : result.effects) {
        switch(effect.type) {
            case BattleActionResult::CHANGE_HEALTH:
                if (effect.target && effect.amount > 0) {
                    effect.target->battleState.currentHealth =
                        std::max(0, effect.target->battleState.currentHealth - effect.amount);
                }
                break;

            case BattleActionResult::CHANGE_HEALTH_END_OF_TURN:
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

            case BattleActionResult::RECOIL:
                if (effect.target && effect.amount > 0) {
                    effect.target->battleState.currentHealth =
                        std::max(0, effect.target->battleState.currentHealth - effect.amount);
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
        QString sourceName = effect.source ? QString::fromStdString(effect.source->pokeState.name) : QString();
        QString targetName = effect.target ? QString::fromStdString(effect.target->pokeState.name) : QString();
        QString sourceRole = effect.source ? (effect.source == m_battleOpponent ? QStringLiteral("opponent") : QStringLiteral("player")) : QString();
        QString targetRole = effect.target ? (effect.target == m_battleOpponent ? QStringLiteral("opponent") : QStringLiteral("player")) : QString();

        switch(effect.type) {
            case BattleActionResult::TEXT:
                sequence.append(createTextAction(QString::fromStdString(effect.text), ms_moveUsedText));
                break;

            case BattleActionResult::CHANGE_HEALTH:
                if (effect.amount > 0) {
                    if (effect.source != nullptr && !sourceRole.isEmpty()) {
                        sequence.append(createAttackAction(sourceRole, ms_attackAnimation));
                    }
                    if (!targetRole.isEmpty()) {
                        sequence.append(createTakeDamageAction(targetRole, ms_takeDamage));
                        sequence.append(createChangeHealthAction(targetRole, -effect.amount, ms_healthChange));
                    }
                }
                break;
            case BattleActionResult::CHANGE_HEALTH_END_OF_TURN:
                if (effect.amount > 0 && !targetRole.isEmpty()) {
                    sequence.append(createSideToSideAction(targetRole, ms_attackAnimation));
                    sequence.append(createTakeDamageAction(targetRole, ms_takeDamage));
                    sequence.append(createChangeHealthAction(targetRole, -effect.amount, ms_healthChange));
                }
                break;

            case BattleActionResult::HEAL:
                if (effect.amount > 0 && !targetRole.isEmpty()) {
                    sequence.append(createTextAction(sourceName + QStringLiteral(" regained health!"), ms_drainEffectText));
                    sequence.append(createChangeHealthAction(targetRole, effect.amount, ms_healthChange));
                }
                break;

            case BattleActionResult::DRAIN:
                if (!sourceRole.isEmpty() && effect.amount > 0) {
                    sequence.append(createTextAction(sourceName + QStringLiteral(" drained health!"), ms_drainEffectText));
                    sequence.append(createChangeHealthAction(sourceRole, effect.amount, ms_healthChange));
                }
                break;

            case BattleActionResult::RECOIL:
                if (!targetRole.isEmpty() && effect.amount > 0) {
                    sequence.append(createTextAction(sourceName + QStringLiteral(" is hit by recoil!"), ms_drainEffectText));
                    sequence.append(createTakeDamageAction(targetRole, ms_takeDamage));
                    sequence.append(createChangeHealthAction(targetRole, -effect.amount, ms_healthChange));
                }
                break;

            case BattleActionResult::CRITICAL:
                sequence.append(createTextAction(QStringLiteral("A critical hit!"), ms_criticalHitText, true));
                break;

            case BattleActionResult::SUPER_EFFECTIVE:
                sequence.append(createTextAction(QStringLiteral("It's super effective!"), ms_effectivenessText, true));
                break;

            case BattleActionResult::NOT_VERY_EFFECTIVE:
                sequence.append(createTextAction(QStringLiteral("It's not very effective..."), ms_effectivenessText, true));
                break;

            case BattleActionResult::NO_EFFECT:
                sequence.append(createTextAction(QStringLiteral("It doesn't affect ") + targetName + QStringLiteral("..."), ms_effectivenessText, true));
                break;

            case BattleActionResult::MISS:
                sequence.append(createTextAction(sourceName + QStringLiteral("'s attack missed!"), ms_statusConditionText));
                break;

            case BattleActionResult::FLINCH:
                sequence.append(createTextAction(sourceName + QStringLiteral(" flinched!"), ms_statusConditionText));
                break;

                if (effect.target) {
                    sequence.append(createSideToSideAction(targetRole, ms_attackAnimation));
                    sequence.append(createStatusCondition(targetRole, effect.ailment, false));
                    QString ailmentText = ailmentToApplicationText(effect.ailment);
                    sequence.append(createTextAction(targetName + QStringLiteral(" ") + ailmentText, ms_statusConditionText));
                }
                break;

            case BattleActionResult::SPLASH:
                sequence.append(createTextAction(QStringLiteral("But nothing happened!"), ms_statusConditionText));
                break;

            case BattleActionResult::STATUS_REMOVED:
                if (effect.target) {
                    sequence.append(createStatusCondition(targetRole, Ailment::Null, true));
                    QString ailment = ailmentToRemovalText(effect.ailment, targetName);
                    sequence.append(createTextAction(ailment, ms_statusConditionText));
                }
                break;

            case BattleActionResult::CONFUSION_ADDED:
                sequence.append(createSideToSideAction(targetRole, ms_attackAnimation));
                sequence.append(createTextAction(targetName + QStringLiteral(" became confused!"), ms_statusConditionText));
                break;

            case BattleActionResult::CONFUSION_REMOVED:
                sequence.append(createTextAction(targetName + QStringLiteral(" snapped out of confusion!"), ms_statusConditionText));
                break;

            case BattleActionResult::CONFUSED_TURN:
                sequence.append(createTextAction(QString::fromStdString(effect.text), ms_moveUsedText));
                sequence.append(createSideToSideAction(sourceRole, ms_attackAnimation));
                break;

            case BattleActionResult::CONFUSION_SELF_HIT:
                if (effect.amount > 0) {
                    sequence.append(createTextAction(QStringLiteral("It hurt itself in its confusion!"), ms_ailmentText));
                    if (!targetRole.isEmpty()) {
                        sequence.append(createTakeDamageAction(targetRole, ms_takeDamage));
                        sequence.append(createChangeHealthAction(targetRole, -effect.amount, ms_healthChange));
                    }
                } else {
                    sequence.append(createTextAction(QStringLiteral("But it had no effect!"), ms_statusConditionText));
                }
                break;

            case BattleActionResult::STAT_CHANGED:
                if (effect.statIndex >= 0 && effect.statIndex < 7) {
                    QString statName = getStatName(effect.statIndex);
                    if(effect.statChange == 1) {
                        sequence.append(createJumpAction(targetRole, ms_attackAnimation));
                        sequence.append(createTextAction(targetName + QStringLiteral("'s ") + statName + QStringLiteral(" rose!"), ms_statusConditionText));
                    } else if(effect.statChange >= 2) {
                        sequence.append(createJumpAction(targetRole, ms_attackAnimation));
                        sequence.append(createTextAction(targetName + QStringLiteral("'s ") + statName + QStringLiteral(" rose sharply!"), ms_statusConditionText));
                    } else if(effect.statChange == -1) {
                        sequence.append(createSideToSideAction(targetRole, ms_attackAnimation));
                        sequence.append(createTextAction(targetName + QStringLiteral("'s ") + statName + QStringLiteral(" fell!"), ms_statusConditionText));
                    } else if(effect.statChange <= -2) {
                        sequence.append(createSideToSideAction(targetRole, ms_attackAnimation));
                        sequence.append(createTextAction(targetName + QStringLiteral("'s ") + statName + QStringLiteral(" harshly fell!"), ms_statusConditionText));
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


QString BattleMoveHandler::ailmentToApplicationText(Ailment ailment){
    switch(ailment) {
        case Ailment::Burn: return QStringLiteral("was burned!");
        case Ailment::Freeze: return QStringLiteral("was frozen solid!");
        case Ailment::Paralysis: return QStringLiteral("is paralyzed!");
        case Ailment::Poison: return QStringLiteral("was poisoned!");
        case Ailment::Toxic: return QStringLiteral("was badly poisoned!");
        case Ailment::Sleep: return QStringLiteral("fell asleep!");
        case Ailment::Confusion: return QStringLiteral("became confused!");
        default: return QString();
    }
};

QString BattleMoveHandler::ailmentToRemovalText(Ailment ailment, const QString& pokemonName){
    switch(ailment) {
        case Ailment::Burn: return pokemonName + QStringLiteral(" is no longer burned!");
        case Ailment::Freeze: return pokemonName + QStringLiteral(" thawed out!");
        case Ailment::Paralysis: return pokemonName + QStringLiteral(" is no longer paralyzed!");
        case Ailment::Poison: return pokemonName + QStringLiteral(" is no longer poisoned!");
        case Ailment::Toxic: return pokemonName + QStringLiteral(" is no longer poisoned!");
        case Ailment::Sleep: return pokemonName + QStringLiteral(" woke up!");
        default: return QString();
    }
};

QString BattleMoveHandler::ailmentToHurtText(Ailment ailment){
    switch(ailment) {
        case Ailment::Burn: return QStringLiteral("burn");
        case Ailment::Poison: return QStringLiteral("poison");
        case Ailment::Toxic: return QStringLiteral("poison");
        default: return QString();
    }
};

QString BattleMoveHandler::getStatName(int statIndex) {
    static const QString statNames[7] = {
        QStringLiteral("attack"),
        QStringLiteral("defense"),
        QStringLiteral("special attack"),
        QStringLiteral("special defense"),
        QStringLiteral("speed"),
        QStringLiteral("accuracy"),
        QStringLiteral("evasion")
    };
    return (statIndex >= 0 && statIndex < 7) ? statNames[statIndex] : QStringLiteral("Stat");
}

const QString BattleMoveHandler::ailmentToLabel(Ailment ailment){
    switch(ailment){
        case Ailment::Burn: return QStringLiteral("BRN");
        case Ailment::Freeze: return QStringLiteral("FRZ");
        case Ailment::Paralysis: return QStringLiteral("PAR");
        case Ailment::Sleep: return QStringLiteral("SLP");
        case Ailment::Poison: return QStringLiteral("PSN");
        case Ailment::Toxic: return QStringLiteral("PSN");
        case Ailment::Null: return QString();
        default: return QString();
    }
}
QVariantMap BattleMoveHandler::createStatusCondition(const QString& role, Ailment ailment, bool remove){
    QVariantMap action;
    action[QStringLiteral("type")] = QStringLiteral("status-condition");
    action[QStringLiteral("role")] = role;
    action[QStringLiteral("remove")] = remove;
    action[QStringLiteral("label")] = ailmentToLabel(ailment);
    return action;
};

QVariantMap BattleMoveHandler::createEndAction() {
    QVariantMap action;
    action[QStringLiteral("type")] = QStringLiteral("end");
    return action;
}

QVariantMap BattleMoveHandler::createTextAction(const QString& message, int delay, bool isAttackTag) {
    QVariantMap action;
    action[QStringLiteral("type")] = QStringLiteral("text");
    action[QStringLiteral("message")] = message;
    action[QStringLiteral("delay")] = delay;
    action[QStringLiteral("isAttackTag")] = isAttackTag;
    return action;
}

QVariantMap BattleMoveHandler::createAttackAction(const QString& role, int delay) {
    QVariantMap action;
    action[QStringLiteral("type")] = QStringLiteral("attack");
    action[QStringLiteral("role")] = role;
    action[QStringLiteral("delay")] = delay;
    return action;
}

QVariantMap BattleMoveHandler::createSideToSideAction(const QString& role, int delay) {
    QVariantMap action;
    action[QStringLiteral("type")] = QStringLiteral("side-to-side");
    action[QStringLiteral("role")] = role;
    action[QStringLiteral("delay")] = delay;
    return action;
}

QVariantMap BattleMoveHandler::createJumpAction(const QString& role, int delay) {
    QVariantMap action;
    action[QStringLiteral("type")] = QStringLiteral("jump-action");
    action[QStringLiteral("role")] = role;
    action[QStringLiteral("delay")] = delay;
    return action;
}

QVariantMap BattleMoveHandler::createTakeDamageAction(const QString& role, int delay) {
    QVariantMap action;
    action[QStringLiteral("type")] = QStringLiteral("take-damage");
    action[QStringLiteral("role")] = role;
    action[QStringLiteral("delay")] = delay;
    return action;
}

QVariantMap BattleMoveHandler::createChangeHealthAction(const QString& role, int amount, int delay) {
    QVariantMap action;
    action[QStringLiteral("type")] = QStringLiteral("change-health");
    action[QStringLiteral("role")] = role;
    action[QStringLiteral("amount")] = amount;
    action[QStringLiteral("delay")] = delay;
    return action;
}
QVariantMap BattleMoveHandler::createCatchAction(int shakes, int delay, int ballId) {
    QVariantMap action;
    action[QStringLiteral("type")] = QStringLiteral("attempt-catch");
    action[QStringLiteral("shakes")] = shakes;
    action[QStringLiteral("delay")] = delay;
    action[QStringLiteral("ballId")] = ballId;
    return action;
}


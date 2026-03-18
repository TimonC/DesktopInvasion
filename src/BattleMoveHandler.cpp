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

void BattleMoveHandler::startActionRound(int actionIndex, QString _action){
    std::string actionStr = _action.toStdString();
    const char* action = actionStr.data();

    assert((!std::strcmp(action, "Switch") || !std::strcmp(action, "Fight") || !std::strcmp(action, "Catch"))
           && "Action must be 'Switch', 'Fight' or 'Catch'");
    assert(actionIndex>-1 && actionIndex<6 && "actionIndex must be between 0 and 5 inclusive");
    assert((!std::strcmp(action,"Switch") || actionIndex<4) && "actionIndex for non-switch action must be between 0 and 3 inclusive");

    std::uniform_int_distribution<int> moveChoiceDist(0, 1);
    int opponentMoveIndex = moveChoiceDist(m_rng);

    const Move* opponentMove = m_battleOpponent->pokeState.moves[opponentMoveIndex];
    Battler* player = m_battleParty[m_chosenIndex];
    const Move* playerMove = player->pokeState.moves[actionIndex];

    m_battleOpponent->delta = {};
    player->delta = {};

    m_battleOpponent->battleState.lastMoveIndex = opponentMoveIndex;
    player->battleState.lastMoveIndex = actionIndex;

    int switchedIn = -1;
    bool playerFirst = true;
    int shakes = -1;

    checkRemoveAilment(*m_battleOpponent);
    checkRemoveAilment(*player);

    if(action[0]=='S'){
       switchedIn = actionIndex;
    } else if(action[0]=='C'){
        shakes = PokeMath::calculateBallShakes(m_rng, m_battleOpponent->pokeState.stats[0], m_battleOpponent->battleState.currentHealth, 50);
    }

    if(action[0]=='F'){
        if (playerMove->priority == opponentMove->priority){
            int oppModifier = m_battleOpponent->battleState.statModifiers[4];
            int oppSpeed = applyStatModifier(m_battleOpponent->pokeState.stats[5], oppModifier);

            int playerModifier = player->battleState.statModifiers[4];
            int playerSpeed = applyStatModifier(player->pokeState.stats[5], playerModifier);

           if(m_battleOpponent->battleState.statusCondition==Ailment::Paralysis) oppSpeed = oppSpeed/2;
           if(player->battleState.statusCondition==Ailment::Paralysis) playerSpeed = playerSpeed/2;

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

        if(playerFirst){
            applyMove(playerMove, player, m_battleOpponent);
            if(m_battleOpponent->battleState.currentHealth > 0 && !m_battleOpponent->delta.flinched) {
                applyMove(opponentMove, m_battleOpponent, player);
            }
            player->delta.flinched = false;
        } else {
            applyMove(opponentMove, m_battleOpponent, player);
            if(player->battleState.currentHealth > 0 && !player->delta.flinched) {
                applyMove(playerMove, player, m_battleOpponent);
            }
            m_battleOpponent->delta.flinched = false;
        }

        applyEndOfTurnEffects(player);
        applyEndOfTurnEffects(m_battleOpponent);

    } else {
        assert(playerFirst && "Player should always go first if it isn't fighting");
        applyMove(opponentMove, m_battleOpponent, player);
        applyEndOfTurnEffects(player);
        applyEndOfTurnEffects(m_battleOpponent);
    }

    QVariantList s = generateActionSequence(*m_battleOpponent, *player, playerFirst, switchedIn, shakes);

    logActionSequence(s);

    emit actionSequenceReady(s);
}


void BattleMoveHandler::checkRemoveAilment(Battler& battler){
    if(battler.battleState.confused==Ailment::Confusion){
        battler.delta.removeConfusion = battler.battleState.confusedCounter>=battler.battleState.confusedTurns;
    }
    if(battler.battleState.statusCondition!=Ailment::Null && battler.battleState.conditionTurns>=0){
        battler.delta.removeStatusCondition = battler.battleState.conditionCounter>=battler.battleState.conditionTurns;
    }

    if(battler.battleState.statusCondition==Ailment::Freeze && battler.battleState.conditionCounter>0){
        battler.delta.removeStatusCondition = PokeMath::calculateFreezeThaw(m_rng);
    }
}

bool BattleMoveHandler::canBattlerMove(Battler* caster) {
    if (caster->battleState.statusCondition == Ailment::Sleep) {
        caster->delta.sleep = true;
        return false;
    }

    if (caster->battleState.statusCondition == Ailment::Freeze) {
        caster->delta.freeze = true;
        return false;
    }

    if (caster->battleState.statusCondition == Ailment::Paralysis) {
        caster->delta.paralyzed = PokeMath::calculateParalysisHit(m_rng);
        return false;
    }

    if (caster->battleState.confused == Ailment::Confusion) {
        std::uniform_int_distribution<int> confuseDist(1, 2);
        if (confuseDist(m_rng) == 1) {
            PokeMath::DamageParams confP;
            confP.lvl = caster->pokeState.lvl;
            confP.power = 40;
            if(caster->battleState.statusCondition==Ailment::Burn) confP.burn = 50;

            int atkModifier = caster->battleState.statModifiers[0];
            int defModifier = caster->battleState.statModifiers[1];
            confP.attack = caster->pokeState.stats[1];
            confP.defense = caster->pokeState.stats[2];
            confP.attack = applyStatModifier(confP.attack, atkModifier);
            confP.defense = applyStatModifier(confP.defense, defModifier);

            int confusionDamage = PokeMath::calculateDamage(confP, m_rng);
            caster->delta.confusedDamage = confusionDamage;
            caster->battleState.currentHealth = std::max(0, caster->battleState.currentHealth - confusionDamage);
            return false;
        }
    }

    if (caster->delta.flinched) {
        return false;
    }

    return true;
}

void BattleMoveHandler::applyEndOfTurnEffects(Battler* battler) {
    if (battler->battleState.statusCondition == Ailment::Burn) {
        int burnDamage = PokeMath::calculateBurnDamage(battler->pokeState.stats[0]);
        battler->delta.ailmentDamage = burnDamage;
        battler->battleState.currentHealth = std::max(0, battler->battleState.currentHealth - burnDamage);
    }

    if (battler->battleState.statusCondition == Ailment::Poison ||  battler->battleState.statusCondition == Ailment::Toxic) {
        int counter = -1;
        if(battler->battleState.statusCondition == Ailment::Toxic) counter = battler->battleState.conditionCounter;
        int poisonDamage = PokeMath::calculatePoisonDamage(battler->pokeState.stats[0], counter);
        battler->delta.ailmentDamage = poisonDamage;
        battler->battleState.currentHealth = std::max(0, battler->battleState.currentHealth - poisonDamage);
    }
}

void BattleMoveHandler::applyMove(const Move* _move, Battler* caster, Battler* target){
    if (!canBattlerMove(caster)) {
        return;
    }

    if (_move->accuracy < 100) {
        std::uniform_int_distribution<int> accuracyDist(1, 100);
        if (accuracyDist(m_rng) > _move->accuracy) {
            caster->delta.miss = true;
            return;
        }
    }

    if(_move->category != MoveCategory::NonDamaging){
        PokeMath::DamageParams params;
        params.lvl = caster->pokeState.lvl;
        params.power = _move->power;

        std::uniform_int_distribution<int> critDist(1, 16/(1+_move->crit_rate));
        bool crit = critDist(m_rng) == 1;
        caster->delta.critical = crit;

        int attackStatIndex = 1;
        if(_move->category == MoveCategory::SpecialAtk){
            attackStatIndex = 3;
        }

        int atkModifier = caster->battleState.statModifiers[attackStatIndex - 1];
        int defModifier = target->battleState.statModifiers[attackStatIndex];

        params.attack = caster->pokeState.stats[attackStatIndex];
        params.defense = target->pokeState.stats[attackStatIndex + 1];
        params.attack = applyStatModifier(params.attack, atkModifier);
        params.defense = applyStatModifier(params.defense, defModifier);

        if(_move->category == MoveCategory::PhysicalAtk){
            if(caster->battleState.statusCondition == Ailment::Burn) params.burn = 50;
        }

        if(crit){
            params.critical = 150;
            if (atkModifier < 0) atkModifier = 0;
            if (defModifier > 0) defModifier = 0;
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
        caster->delta.damage = damage;

        target->battleState.currentHealth = std::max(0, target->battleState.currentHealth - damage);

        int combinedEffectiveness = params.type1 * params.type2;
        if (combinedEffectiveness > 10000) {
            caster->delta.superEffective = true;
        } else if (combinedEffectiveness<10000 && combinedEffectiveness > 0) {
            caster->delta.notVeryEffective = true;
        } else if (combinedEffectiveness==0){
            caster->delta.noEffect = true;
        }

        if (_move->drain > 0) {
            int drainAmount = damage * _move->drain / 100;
            caster->delta.drain = drainAmount;
            caster->battleState.currentHealth = std::min(100, caster->battleState.currentHealth + drainAmount);
        }

        if (_move->healing > 0) {
            int healAmount = caster->pokeState.stats[0] * _move->healing / 100;
            caster->delta.heal = healAmount;
            caster->battleState.currentHealth = std::min(100, caster->battleState.currentHealth + healAmount);
        }
    }
    applySecondaryEffects(_move, target);
}

void BattleMoveHandler::applySecondaryEffects(const Move* _move, Battler* target) {
    bool ailmentApplied = true;
    bool statApplied = true;

    if(_move->category!=MoveCategory::NonDamaging){
        std::uniform_int_distribution<int> effectDist(1, 100);
        ailmentApplied = effectDist(m_rng) <= _move->ailment_chance;
        statApplied = effectDist(m_rng) <= _move->stat_chance;
    }

    if (ailmentApplied && _move->ailment != Ailment::Null) {
        if(_move->ailment==Ailment::Confusion){
            if(target->battleState.confused == Ailment::Confusion && _move->category==MoveCategory::NonDamaging) {
                target->delta.failedConfusion = true;
            } else {
                target->delta.addConfusion = true;
                target->battleState.confused = Ailment::Confusion;
                target->battleState.confusedTurns = PokeMath::calculateAilmentTurns(Ailment::Confusion, m_rng);
                target->battleState.confusedCounter = 0;
            }
        }else{
            if(target->battleState.statusCondition != Ailment::Null && _move->category==MoveCategory::NonDamaging) {
                target->delta.failedStatusCondition = true;
            } else {
                target->delta.addStatusCondition = _move->ailment;
                target->battleState.statusCondition = _move->ailment;
                target->battleState.conditionCounter = 0;
                target->battleState.conditionTurns = PokeMath::calculateAilmentTurns(_move->ailment, m_rng);
            }
        }
    }

    if (_move->flinch_chance > 0) {
        std::uniform_int_distribution<int> flinchDist(1, 100);
        if (flinchDist(m_rng) <= _move->flinch_chance) {
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
                        target->delta.statMaxed[i] = true;
                    } else {
                        target->delta.statMinned[i] = true;
                    }
                } else {
                    target->delta.deltaStatModifiers[i] = actualChange;
                    target->battleState.statModifiers[i] = clampedModifier;
                }
            }
        }
    }
}

int BattleMoveHandler::applyStatModifier(int baseStat, int modifier) {
    if (modifier == 0) return baseStat;

    static const float multiplier[13] = {
        0.25f, 0.28f, 0.33f, 0.40f, 0.50f, 0.66f,
        1.0f,
        1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f
    };

    int index = modifier + 6;
    return static_cast<int>(baseStat * multiplier[index]);
}

void BattleMoveHandler::generateMoveSequence(QVariantList& s, Battler& attacker, Battler& defender, bool isAttackerPlayer) {
    QString attackerName = QString::fromStdString(attacker.pokeState.name);
    QString defenderName = QString::fromStdString(defender.pokeState.name);
    QString attackerRole = isAttackerPlayer ? "player" : "opponent";
    QString defenderRole = isAttackerPlayer ? "opponent" : "player";

    const Move* _move = attacker.pokeState.moves[attacker.battleState.lastMoveIndex];
    QString moveName = QString::fromStdString(_move->name);

    if(attacker.delta.flinched) {
        s.append(createTextAction(attackerName + " flinched!", ms_statusConditionText));
    } else if(attacker.delta.sleep) {
        s.append(createTextAction(attackerName + " is fast asleep!", ms_statusConditionText));
    } else if(attacker.delta.freeze) {
        s.append(createTextAction(attackerName + " is frozen solid!", ms_statusConditionText));
    } else if(attacker.delta.paralyzed) {
        s.append(createTextAction(attackerName + " is paralyzed! It can't move!", ms_statusConditionText));
    }

    else{
        if(attacker.battleState.confused==Ailment::Confusion && !(attacker.delta.addConfusion && attacker.delta.isFirst)){
            s.append(createTextAction(attackerName + " is confused!", ms_ailmentText));
        }
        if(attacker.delta.confusedDamage > 0) {
            s.append(createTextAction("It hurt itself in its confusion!", ms_ailmentText));
            s.append(createDamageAction(attackerRole, ms_damageAnimation));
            s.append(createHealthChangeAction(attackerRole, -attacker.delta.confusedDamage, ms_healthChange));
        } else if(attacker.delta.miss) {
            s.append(createTextAction(attackerName + " used " + moveName + "!", ms_moveUsedText));
            s.append(createTextAction(attackerName + "'s attack missed!", ms_statusConditionText));
        } else if(attacker.delta.damage > 0) {
            s.append(createTextAction(attackerName + " used " + moveName + "!", ms_moveUsedText));
            s.append(createAttackAction(attackerRole, ms_attackAnimation));
            s.append(createDamageAction(defenderRole, ms_damageAnimation));
            s.append(createHealthChangeAction(defenderRole, -attacker.delta.damage, ms_healthChange));

            if(attacker.delta.critical) {
                s.append(createTextAction("A critical hit!", ms_criticalHitText));
            }
            if(attacker.delta.superEffective) {
                s.append(createTextAction("It's super effective!", ms_effectivenessText));
            } else if (attacker.delta.notVeryEffective) {
                s.append(createTextAction("It's not very effective...", ms_effectivenessText));
            }
            if(attacker.delta.drain > 0) {
                s.append(createTextAction(attackerName + " drained health!", ms_drainEffectText));
                s.append(createHealthChangeAction(attackerRole, attacker.delta.drain, ms_healthChange));
            }
        }else if(attacker.delta.noEffect){
                s.append(createTextAction(attackerName + " used " + moveName + "!", ms_moveUsedText));
                s.append(createTextAction("It doesn't affect " + defenderName + "...", ms_effectivenessText));
        }else{
                s.append(createTextAction(attackerName + " used " + moveName + "!", ms_moveUsedText));
        }
    }

    addPostMoveEffects(s, defender, defenderName, !isAttackerPlayer);
}


QVariantList BattleMoveHandler::generateActionSequence(Battler& opponent, Battler& player, bool playerFirst, int switchedIn, int shakes){
    QVariantList s;

    if(shakes > -1) {
        QString playerName = QString::fromStdString(player.pokeState.name);
        std::uniform_int_distribution<int> shakeDist(0, 1);
        shakes = shakeDist(m_rng) == 0 ? 1 : 4;
        s.append(createTextAction("Player used one Poké Ball!", 300));
        s.append(createCatchAction(shakes, ms_catchStart));

        generateMoveSequence(s, opponent, player, false);

        QString opponentName = QString::fromStdString(opponent.pokeState.name);
        addEndOfTurnEffects(s, opponent, opponentName, false);
        addEndOfTurnEffects(s, player, playerName, true);

        s.append(createEndAction());
        return s;
    }

    if(switchedIn > -1) {
        generateMoveSequence(s, opponent, player, false);

        QString opponentName = QString::fromStdString(opponent.pokeState.name);
        QString playerName = QString::fromStdString(player.pokeState.name);
        addEndOfTurnEffects(s, opponent, opponentName, false);
        addEndOfTurnEffects(s, player, playerName, true);

        s.append(createEndAction());
        return s;
    }

    Battler& firstAttacker = playerFirst ? player : opponent;
    Battler& firstDefender = playerFirst ? opponent : player;
    Battler& secondAttacker = playerFirst ? opponent : player;
    Battler& secondDefender = playerFirst ? player : opponent;

    generateMoveSequence(s, firstAttacker, firstDefender, playerFirst);

    if(firstDefender.battleState.currentHealth > 0) {
        generateMoveSequence(s, secondAttacker, secondDefender, !playerFirst);
    }

    QString opponentName = QString::fromStdString(opponent.pokeState.name);
    QString playerName = QString::fromStdString(player.pokeState.name);
    addEndOfTurnEffects(s, opponent, opponentName, false);
    addEndOfTurnEffects(s, player, playerName, true);

    s.append(createEndAction());

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

void BattleMoveHandler::addPostMoveEffects(QVariantList& s, Battler& battler, const QString& name, bool isPlayer) {
    QString role = isPlayer ? "player" : "opponent";

    if(battler.delta.heal > 0) {
        s.append(createTextAction(name + " regained health!", ms_ailmentText));
        s.append(createHealthChangeAction(role, battler.delta.heal, ms_healthChange));
    }

    if(battler.delta.addStatusCondition != Ailment::Null) {
        s.append(createStatusCondition(role, battler.delta.addStatusCondition));
        QString ailmentText = ailmentToApplicationText(battler.delta.addStatusCondition);
        s.append(createTextAction(name + " " + ailmentText, ms_statusConditionText));
    }

    if(battler.delta.failedStatusCondition) {
        s.append(createTextAction("But it failed!", ms_statusConditionText));
    }

    if(battler.delta.addConfusion) {
        s.append(createTextAction(name + " became confused!", ms_statusConditionText));
    }

    if(battler.delta.failedConfusion) {
        s.append(createTextAction("But it failed!", ms_statusConditionText));
    }

    if(battler.delta.removeStatusCondition) {
        s.append(createStatusCondition(role, Ailment::Null));
        QString ailment = ailmentToRemovalText(battler.battleState.statusCondition);
        s.append(createTextAction(name + " is no longer " + ailment + "!", ms_statusConditionText));
        battler.battleState.statusCondition  = Ailment::Null;
        battler.battleState.conditionCounter = -1;
        battler.battleState.conditionTurns = -1;
    }

    if(battler.delta.removeConfusion) {
        s.append(createTextAction(name + " snapped out of confusion!", ms_statusConditionText));
        battler.battleState.confused = Ailment::Null;
        battler.battleState.confusedCounter = -1;
        battler.battleState.confusedTurns = -1;
    }

    for (int i = 0; i < 5; i++) {
        int change = battler.delta.deltaStatModifiers[i];

        if (change != 0) {
            QString statName = getStatName(i);

            if(change == 1) {
                s.append(createTextAction(name + "'s " + statName + " rose!", ms_statusConditionText));
            } else if(change >= 2) {
                s.append(createTextAction(name + "'s " + statName + " rose sharply!", ms_statusConditionText));
            } else if(change == -1) {
                s.append(createTextAction(name + "'s " + statName + " fell!", ms_statusConditionText));
            } else if(change <= -2) {
                s.append(createTextAction(name + "'s " + statName + " harshly fell!", ms_statusConditionText));
            }
        }

        if(battler.delta.statMaxed[i]) {
            QString statName = getStatName(i);
            s.append(createTextAction(name + "'s " + statName + " won't go any higher!", ms_statusConditionText));
        }

        if(battler.delta.statMinned[i]) {
            QString statName = getStatName(i);
            s.append(createTextAction(name + "'s " + statName + " won't go any lower!", ms_statusConditionText));
        }
    }
}

void BattleMoveHandler::addEndOfTurnEffects(QVariantList& s, Battler& battler, const QString& name, bool isPlayer) {
    QString role = isPlayer ? "player" : "opponent";

    if(battler.delta.ailmentDamage > 0) {
        QString ailment = ailmentToHurtText(battler.battleState.statusCondition);
        s.append(createTextAction(name + " is hurt by its " + ailment + "!", ms_statusConditionText));
        s.append(createDamageAction(role, ms_damageAnimation));
        s.append(createHealthChangeAction(role, -battler.delta.ailmentDamage, ms_healthChange));
    }

   if(battler.battleState.confused==Ailment::Confusion) battler.battleState.confusedCounter++;
   if(battler.battleState.statusCondition!=Ailment::Null) battler.battleState.conditionCounter++;
}

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

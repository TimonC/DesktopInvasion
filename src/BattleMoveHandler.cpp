#include "PokeMath/calculatePokeStats.h"
#include "data_move.h"
#include <BattleMoveHandler.h>
#include <cstring>
#include <globals.h>
#include <PokeMath/processCatchAttempt.h>
#include <PokeMath/calculateDamage.h>

BattleMoveHandler::BattleMoveHandler(Battler battleOpponent, Battler battleParty[6])
    : m_battleOpponent(battleOpponent)
    , m_rng(std::random_device{}())
{
    std::copy(battleParty, battleParty + 6, m_battleParty);

};

void BattleMoveHandler::startActionRound(int actionIndex, const char* action){
    assert((!std::strcmp(action, "Switch") || !std::strcmp(action, "Fight") || !std::strcmp(action, "Catch"))
           && "Action must be 'Switch', 'Fight' or 'Catch'");
    assert(actionIndex>-1 && actionIndex<6 && "actionIndex must be between 0 and 5 inclusive");
    assert((!std::strcmp(action,"Switch") || actionIndex<4) && "actionIndex for non-switch action must be between 0 and 3 inclusive");
        //TODO add catch rate in poke, add accuracy mod in move, add xp calculations...


    int opponentMoveIndex = rand()%4;

    const Move* playerMove = m_battleParty[m_chosenPartyIndex].pokeState.moves[actionIndex];
    const Move* opponentMove = m_battleOpponent.pokeState.moves[opponentMoveIndex];

    if(action[0]=='S'){
       m_chosenPartyIndex = actionIndex;
    }else if(action[0]=='C'){
        int shakes = processCatchAttempt(m_rng, m_battleOpponent.pokeState.stats[0], m_battleOpponent.battleState.currentHealth, 50);
        if(shakes>3){
        }
    }

    bool playerFirst = true;
    if(action[0]=='F'){
        if (playerMove->priority == opponentMove->priority){
           playerFirst = m_battleOpponent.pokeState.stats[5] < m_battleParty[m_chosenPartyIndex].pokeState.stats[5];
        }else{
           playerFirst = opponentMove->priority < playerMove->priority;
        }
        if(playerFirst){
            applyMove(playerMove, m_battleParty[m_chosenPartyIndex], m_battleOpponent);
        }
    }

    applyMove(opponentMove, m_battleOpponent, m_battleParty[m_chosenPartyIndex]);
    if(!playerFirst){
        applyMove(playerMove, m_battleParty[m_chosenPartyIndex], m_battleOpponent);
    }

    emit actionRoundOver(m_battleOpponent.battleState, m_battleParty[m_chosenPartyIndex].battleState);
};

void BattleMoveHandler::applyMove(const Move* _move, Battler& caster, Battler& target){
    /* _move.accuracy */
    std::uniform_int_distribution<int> dist(1, 16/(1+_move->crit_rate));
    bool crit = dist(m_rng) == 1;

    if(_move->category!=MoveCategory::NonDamaging){
        DamageParams p;
        p.lvl = caster.pokeState.lvl;
        p.power = _move->power;

        int atkIndex = 1;
        if(_move->category==MoveCategory::PhysicalAtk){
            if(caster.battleState.statusCondition==Ailment::Burn) p.burn = 50;
        }else{
            atkIndex+=1;
        }
        p.attack = caster.pokeState.stats[atkIndex];
        p.defense = target.pokeState.stats[atkIndex+2];

        int atkModifier = caster.battleState.statModifiers[atkIndex];
        int defModifier = caster.battleState.statModifiers[atkIndex+2];
        if(crit){
            p.critical = 150; //gen 5+ is 150, gen 4 is 200, I like it a bit more balanced
            std::max(0, atkModifier);
            std::min(0, defModifier);
        }
        p.attack = applyStatModifier(p.attack, atkModifier);
        p.defense = applyStatModifier(p.attack, defModifier);
    }

};


#include <BattleMoveHandler.h>
#include "PokeMath/calculatePokeStats.h"
#include "data_move.h"
#include <cstring>
#include <globals.h>
#include <PokeMath/processCatchAttempt.h>
#include <PokeMath/calculateDamage.h>

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
    battler->pokeState.lvl = state.lvl;
    battler->pokeState.stats = calculatePokeStats(
        state.lvl,
        poke->base_stats,
        state.ivs,
        state.evs,
        getNatureMultipliers(state.nature)
    );
    battler->pokeState.types[0] = &poke->types[0];
    battler->pokeState.types[1] = &poke->types[1];

    for (int i = 0; i < 4; ++i) {
        battler->pokeState.moves[i] = Globals::getMove(state.moves[i]);
    }

    return battler;
}


void BattleMoveHandler::startActionRound(int actionIndex, const char* action){
    assert((!std::strcmp(action, "Switch") || !std::strcmp(action, "Fight") || !std::strcmp(action, "Catch"))
           && "Action must be 'Switch', 'Fight' or 'Catch'");
    assert(actionIndex>-1 && actionIndex<6 && "actionIndex must be between 0 and 5 inclusive");
    assert((!std::strcmp(action,"Switch") || actionIndex<4) && "actionIndex for non-switch action must be between 0 and 3 inclusive");

    BattleStateDelta opponentStateDelta;
    BattleStateDelta playerStateDelta;

    int opponentMoveIndex = rand()%4;

    const Move* opponentMove = m_battleOpponent->pokeState.moves[opponentMoveIndex];
    const Move* playerMove = m_battleParty[m_chosenPartyIndex]->pokeState.moves[actionIndex];
    //Wipe battle state delta
    m_battleOpponent->delta = {};
    m_battleParty[m_chosenPartyIndex]->delta = {};

    m_battleOpponent->battleState.lastMoveIndex = opponentMoveIndex;
    m_battleOpponent->battleState.lastMoveIndex = actionIndex; //this property is ignored on non-fight actions, always passing it is a bit of a hack

    int switchedIn = -1;
    bool playerFirst = true;
    if(action[0]=='S'){
       m_chosenPartyIndex = actionIndex;
       switchedIn = actionIndex;
    }else if(action[0]=='C'){
        int shakes = processCatchAttempt(m_rng, m_battleOpponent->pokeState.stats[0], m_battleOpponent->battleState.currentHealth, 50);
        if(shakes>3){
            emit actionRoundOver(*m_battleOpponent, *m_battleParty[m_chosenPartyIndex], playerFirst, switchedIn, shakes);
        }
    }

    if(action[0]=='F'){
        if (playerMove->priority == opponentMove->priority){
           playerFirst = m_battleOpponent->pokeState.stats[5] < m_battleParty[m_chosenPartyIndex]->pokeState.stats[5];
        }else{
           playerFirst = opponentMove->priority < playerMove->priority;
        }
        if(playerFirst){
            applyMove(playerMove, m_battleParty[m_chosenPartyIndex], m_battleOpponent);
        }
    }else{
        assert(playerFirst && "Player should always go first if it isn't fighting");
    }

    applyMove(opponentMove, m_battleOpponent, m_battleParty[m_chosenPartyIndex]);
    if(!playerFirst){
        applyMove(playerMove, m_battleParty[m_chosenPartyIndex], m_battleOpponent);
    }

    emit actionRoundOver(*m_battleOpponent, *m_battleParty[m_chosenPartyIndex], playerFirst, switchedIn);
}

void BattleMoveHandler::applyMove(const Move* _move, Battler* caster, Battler* target){
    std::uniform_int_distribution<int> dist(1, 16/(1+_move->crit_rate));
    bool crit = dist(m_rng) == 1;

    if(_move->category!=MoveCategory::NonDamaging){
        DamageParams p;
        p.lvl = caster->pokeState.lvl;
        p.power = _move->power;

        int atkIndex = 1;
        if(_move->category==MoveCategory::PhysicalAtk){
            if(caster->battleState.statusCondition==Ailment::Burn) p.burn = 50;
        }else{
            atkIndex+=1;
        }
        p.attack = caster->pokeState.stats[atkIndex];
        p.defense = target->pokeState.stats[atkIndex+2];

        int atkModifier = caster->battleState.statModifiers[atkIndex];
        int defModifier = caster->battleState.statModifiers[atkIndex+2];
        if(crit){
            p.critical = 150;
            std::max(0, atkModifier);
            std::min(0, defModifier);
        }
        p.attack = applyStatModifier(p.attack, atkModifier);
        p.defense = applyStatModifier(p.attack, defModifier);
    }

}

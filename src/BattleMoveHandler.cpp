#include "data_move.h"
#include <BattleMoveHandler.h>
#include <cstring>
#include <globals.h>
#include <PokeMath/processCatchAttempt.h>

BattleMoveHandler::BattleMoveHandler(Battler battleOpponent, Battler battleParty[6])
    : m_battleOpponent(battleOpponent)
{
    std::copy(battleParty, battleParty + 6, m_battleParty);
};

void BattleMoveHandler::startActionRound(int actionIndex, const char* action){
   assert((!std::strcmp(action, "Switch") || !std::strcmp(action, "Fight") || !std::strcmp(action, "Catch"))
           && "Action must be 'Switch', 'Fight' or 'Catch'");
   assert(actionIndex>-1 && actionIndex<6 && "actionIndex must be between 0 and 5 inclusive");
   assert((!std::strcmp(action,"Switch") || actionIndex<4) && "actionIndex for non-switch action must be between 0 and 3 inclusive");

   const Move* playerMove = nullptr;
   bool opponentFirst = rand() > 0.5;

   if(action[0]=='S'){
       m_chosenPartyIndex = actionIndex;
   }else if(action[0]=='C'){

   }


    const Move* opponentMove = m_battleOpponent.pokeState.moves[rand()%4];

    if(!playerMove || opponentFirst){
        playerMove = m_battleParty[m_chosenPartyIndex].pokeState.moves[actionIndex];
        applyMove(opponentMove, m_battleOpponent, m_battleParty[m_chosenPartyIndex]);
    }
};

int BattleMoveHandler::attemptCatch(int pokeBallId){
    if(pokeBallId==3){
        return 3;
    }else{
        int shakeCount = 0;
        do{
        }while{
            shakeCount<3;
        }
        return 3;
    }
};
void BattleMoveHandler::applyMove(const Move* _move, Battler& caster, Battler& target){
    DamageParams p;
    p.level = caster.pokeState.lvl;
};

/* https://bulbapedia.bulbagarden.net/wiki/Damage */
const int BattleMoveHandler::calculateDamage(const DamageParams& params) {
    float random = 1.0f - (std::rand() % 16) / 100.0f;
    return static_cast<int>(
        (params.power * params.attack / (50.0f * params.defense) *
        (2.0f + 0.4f * params.level) * params.burn + 2.0f) *
        params.critical * random * params.stab *
        params.type1 * params.type2
    );
}

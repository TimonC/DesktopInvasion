#include "data_move.h"
#include <BattleMoveHandler.h>
#include <globals.h>

BattleMoveHandler::BattleMoveHandler(Poke opponent, Poke party[6])
    : m_opponent(opponent)
{
    std::copy(party, party + 6, m_party),
    m_weatherCondition = WeatherCondition::Clear;
};

void BattleMoveHandler::startActionRound(int playerMoveIndex){
   const Move* playerMove = Globals::getMove(m_party[m_chosenPartyIndex].pokeStatic.moves[playerMoveIndex]);
   const Move* opponentMove = Globals::getMove(m_opponent.pokeStatic.moves[std::rand()%4]);

   int playerFirst = rand()>0.5;
   Poke& player = m_party[m_chosenPartyIndex];

   if(playerFirst){
       applyMove(playerMove, player, m_opponent);
       applyMove(opponentMove, m_opponent, player);
    }else{
       applyMove(opponentMove, m_opponent, player);
       applyMove(playerMove, player, m_opponent);
    }
};

void BattleMoveHandler::applyMove(const Move* _move, Poke& caster, Poke& target){
    int statCategoryId=-1;
    int atk = caster.pokeStatic.stats[statCategoryId];
    int def = target.pokeStatic.stats[statCategoryId + 2];
};


int BattleMoveHandler::calculateDamage(){
    int pLevel = 1;
    int pPower = 1;
    int pA = 1;
    int pD = 1;

    int pBurn = 1;
    int pScreen = 1;
    int pTargets = 1;

    int pWeather = 1;
    int pFF = 1;

    int pStockpile = 1;
    int pCritical = 1;
    int pDoubleDmg = 1;
    int pCharge = 1;
    int pHH = 1;

    int pSTAB = 1;
    int pType1 = 1;
    int pType2 = 1;

    int pRandom = 1;

    return 1;
};

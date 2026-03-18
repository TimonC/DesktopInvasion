#include "data_move.h"
#include <BattleMoveHandler.h>
#include <globals.h>

BattleMoveHandler::BattleMoveHandler(Battler opponent, Battler party[6])
    : m_opponent(opponent)
{
    std::copy(party, party + 6, m_party);
};

void BattleMoveHandler::startActionRound(int playerMoveIndex){
   const Move* playerMove = Globals::getMove(m_party[m_chosenPartyIndex].pokeStatic.moves[playerMoveIndex]);
   const Move* opponentMove = Globals::getMove(m_opponent.pokeStatic.moves[std::rand()%4]);

   int playerFirst = rand()>0.5;
   Battler& player = m_party[m_chosenPartyIndex];


};

void BattleMoveHandler::applyMove(const Move* _move, Battler& caster, Battler& target){
    int statCategoryId=-1;
    int atk = caster.pokeStatic.stats[statCategoryId];
    int def = target.pokeStatic.stats[statCategoryId + 2];

    int rLevel = 1;
    int rPower = 1;
    int rA = 1;
    int rD = 1;
/* https://bulbapedia.bulbagarden.net/wiki/Damage */
    _move->power * rA/(50*rD) * (2 + 2/5*rLevel);
};


int BattleMoveHandler::calculateDamage(){
/* https://bulbapedia.bulbagarden.net/wiki/Damage */

    int rBurn = 1;
    /* int rScreen = 1; */
    int rTargets = 1;

    /* int rWeather = 1; */
    /* int rFF = 1; */

    int rStockpile = 1;
    int rCritical = 1;
    int rDoubleDmg = 1;
    int rCharge = 1;
    int rHH = 1;

    int rSTAB = 1;
    int rType1 = 1;
    int rType2 = 1;

    int rRandom = 1;


    return 1;
};

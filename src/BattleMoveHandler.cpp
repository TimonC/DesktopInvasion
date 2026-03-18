#include "data_battle.h"
#include <BattleMoveHandler.h>
#include <algorithm>
#include <globals.h>

BattleMoveHandler::BattleMoveHandler(Poke opponent, Poke party[6])
    : m_opponent(opponent)
{
    std::copy(party, party + 6, m_party),
    m_weatherCondition = WeatherCondition::Clear;
};

void BattleMoveHandler::startActionRound(int playerMoveIndex){
   Move playerMove = Globals::getMove(m_party[m_chosenPartyIndex].pokeStatic.moves[playerMoveIndex]);
   Move opponentMove = Globals::getMove(m_opponent.pokeStatic.moves[std::rand()%4]);

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

void BattleMoveHandler::applyMove(Move _move, Poke& caster, Poke& target){
    int statCategoryId=-1;
    switch (_move.moveCategory){
        case MoveCategory::Physical:
            statCategoryId = 1;
            break;
        case MoveCategory::Special:
            statCategoryId = 1;
            break;
        case MoveCategory::Status:
            break;
    }

    int atk = caster.pokeStatic.stats[statCategoryId];
    int def = target.pokeStatic.stats[statCategoryId + 2];



};




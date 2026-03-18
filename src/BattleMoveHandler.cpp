#include "data_battle.h"
#include <BattleMoveHandler.h>
#include <algorithm>

BattleMoveHandler::BattleMoveHandler(Poke opponent, Poke party[6]) : m_opponent(opponent) {

    std::copy(party, party + 6, m_party),
    m_weatherCondition = WeatherCondition::Clear;
};

void BattleMoveHandler::startActionRound(int playerMoveIndex){};




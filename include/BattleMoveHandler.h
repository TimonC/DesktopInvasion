#ifndef BATTLEMOVEHANDLER_H
#define BATTLEMOVEHANDLER_H

#include <QObject>
#include <data_battle.h>


class BattleMoveHandler : public QObject{
    Q_OBJECT

public:
    BattleMoveHandler(Poke opponent, Poke party[6]);
    void startActionRound(int playerMoveIndex);

signals:
    void actionRoundOver(State& opponentState, State& chosenState);

private:
    Poke m_opponent;
    Poke m_party[6];
    int m_chosenPartyIndex = 0;
    int m_partyPokemonSentOut[6] = {-1,-1,-1,-1,-1,-1};

    WeatherCondition m_weatherCondition;
};

#endif

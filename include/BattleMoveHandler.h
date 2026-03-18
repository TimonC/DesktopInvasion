#ifndef BATTLEMOVEHANDLER_H
#define BATTLEMOVEHANDLER_H

#include <QObject>
#include <data_battle.h>

struct Static{
    int stats[6];
    Type types[2];
    Move moves[4];
};

struct State{
    int currentHealth;
    StatusCondition conditions[10];
};

struct Poke{
    Static pokeStatic;
    State pokeState;
    void applyMove(int moveIndex);
};

class BattleMoveHandler : public QObject{
    Q_OBJECT

public:
    BattleMoveHandler(int opponentId, int partyIds[6]);
    void setChosenPoke(int partyIndex);
    void startActionRound(int playerMoveIndex);

signals:
    void actionRoundOver(State& opponentState, State& chosenState);

private:
    WeatherCondition weatherCondition;
    Poke opponent;
    Poke& chosen;
    Poke party[6];
    int partyPokemonSentOut[6];
};

#endif

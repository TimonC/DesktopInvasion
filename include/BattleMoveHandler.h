#ifndef BATTLEMOVEHANDLER_H
#define BATTLEMOVEHANDLER_H

#include <move_data.h>

struct Static{
    int id;
    Type types[2];
    int stats[6];
    Move moves[4];
};

struct State{
    int currentHealth;
    StatusCondition conditions[10];
};

struct Poke{
    Static pokeStatic;
    State pokeState;
};

class BattleMoveHandler{
public:
    BattleMoveHandler(int opponentId, int partyIds[6]);
private:
    WeatherCondition weatherCondition;
    Poke opponent;
    Poke chosen;
};

#endif

#ifndef BATTLEMOVEHANDLER_H
#define BATTLEMOVEHANDLER_H

#include <QObject>
#include <data_move.h>
#include <gamestate.h>

struct Static{
    int uid;
    int stats[6]; //HP, Atk, SpAtk, Def, SpDef, Spd
    Type types[2];
    int moves[4];
};

struct State{
    int currentHealthRatio = 1;
    Ailment statusCondition = Ailment::Null;
    Ailment confused = Ailment::Null;
    std::array<int, 5> stat_changes = {0, 0, 0, 0, 0}; //Atk, SpAtk, Def, SpDef, Spd
};

struct Battler{
    Static pokeStatic;
    State battleState;
};

class BattleMoveHandler : public QObject{
    Q_OBJECT

public:
    BattleMoveHandler(Battler opponent, Battler party[6]);
    void startActionRound(int playerMoveIndex);

signals:
    void actionRoundOver(State& opponentState, State& chosenState);

private:
    void applyMove(const Move* moveToApply, Battler& caster, Battler& target);
    Battler m_opponent;
    Battler m_party[6];
    int m_chosenPartyIndex = 0;
    int m_partyPokemonSentOut[6] = {-1,-1,-1,-1,-1,-1};

    int calculateDamage();
};

#endif

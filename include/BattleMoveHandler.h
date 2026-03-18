#ifndef BATTLEMOVEHANDLER_H
#define BATTLEMOVEHANDLER_H

#include <QObject>
#include <data_move.h>
#include <gamestate.h>

struct Static{
    int uid;
    std::array<int, 6> stats; //HP, Atk, SpAtk, Def, SpDef, Spd
    const Type* types[2];
    const Move* moves[4];
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

struct DamageParams {
    int level = 1;
    int power = 1;
    int attack = 1;
    int defense = 1;
    float burn = 1.0f;
    float critical = 1.0f;
    float stab = 1.0f;
    float type1 = 1.0f;
    float type2 = 1.0f;
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

    const int calculateDamage(const DamageParams& params);
};

#endif

#ifndef BATTLEMOVEHANDLER_H
#define BATTLEMOVEHANDLER_H

#include <QObject>
#include <data_move.h>
#include <gamestate.h>
#include <qtmetamacros.h>
#include <random>

struct PokeState{
    int lvl;
    std::array<int, 6> stats; //HP, Atk, SpAtk, Def, SpDef, Spd
    const Type* types[2];
    const Move* moves[4];
};

struct BattleState{
    int currentHealth = 100;
    Ailment statusCondition = Ailment::Null;
    Ailment confused = Ailment::Null;
    std::array<int, 5> statModifiers = {0, 0, 0, 0, 0}; //Atk, SpAtk, Def, SpDef, Spd
};

struct Battler{
    PokeState pokeState;
    BattleState battleState;
};

class BattleMoveHandler : public QObject{
    Q_OBJECT

public:
    BattleMoveHandler(Battler battleOpponent, Battler battleParty[6]);

signals:
    void actionRoundOver(BattleState& opponentState, BattleState& playerState);

public slots:
    void startActionRound(int playerMoveIndex, const char* action);

private:
    void applyMove(const Move* moveToApply, Battler& caster, Battler& target);
    Battler m_battleOpponent;
    Battler m_battleParty[6];
    int m_chosenPartyIndex = 0;
    int m_partyPokemonSentOut[6] = {-1,-1,-1,-1,-1,-1};
    std::mt19937 m_rng;

};

#endif

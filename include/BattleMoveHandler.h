#ifndef BATTLEMOVEHANDLER_H
#define BATTLEMOVEHANDLER_H

#include <QObject>
#include <data_move.h>
#include <gamestate.h>
#include <qtmetamacros.h>

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
    std::array<int, 5> stat_changes = {0, 0, 0, 0, 0}; //Atk, SpAtk, Def, SpDef, Spd
};

struct Battler{
    PokeState pokeState;
    BattleState battleState;
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
    BattleMoveHandler(Battler battleOpponent, Battler battleParty[6]);

signals:
    void actionRoundOver(BattleState& opponentState, BattleState& playerState);

public slots:
    void startActionRound(int playerMoveIndex, const char* action);

private:
    void applyMove(const Move* moveToApply, Battler& caster, Battler& target);
    int attemptCatch(int pokeBallId);
    Battler m_battleOpponent;
    Battler m_battleParty[6];
    int m_chosenPartyIndex = 0;
    int m_partyPokemonSentOut[6] = {-1,-1,-1,-1,-1,-1};

    const int calculateDamage(const DamageParams& params);
};

#endif

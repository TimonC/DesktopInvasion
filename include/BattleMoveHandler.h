#ifndef BATTLEMOVEHANDLER_H
#define BATTLEMOVEHANDLER_H

#include <QObject>
#include <data_move.h>
#include <gamestate.h>
#include <qtmetamacros.h>
#include <random>
#include <array>


struct BattleStateDelta{
    bool switchedIn = false;
    int damage = 0;
    int drain = 0;
    int heal = 0;
    std::array<int, 5> deltaStatModifiers = {0, 0, 0, 0, 0};
    bool miss = false;

    bool superEffective = false;
    bool critical = false;

    bool flinched = false;
    bool sleep = false;
    bool paralyzed = false;
    bool freeze = false;
    int ailmentDaamge = -1;
    int confusedDamage = -1;

    Ailment addStatusCondition = Ailment::Null;
    Ailment removeStatusCondition = Ailment::Null;

    bool addConfusion = false;
    bool removeConfusion = false;
};

struct PokeState{
    int lvl;
    std::array<int, 6> stats;
    const Type* types[2];
    const Move* moves[4];
    int xpForWinner = 100;
};

struct BattleState{
    int currentHealth = 100;
    Ailment statusCondition = Ailment::Null;
    Ailment confused = Ailment::Null;
    std::array<int, 5> statModifiers = {0, 0, 0, 0, 0};
    int lastMoveIndex = -1;
};

struct Battler{
    PokeState pokeState;
    BattleState battleState;
    BattleStateDelta delta;
};

class BattleMoveHandler : public QObject{
    Q_OBJECT

public:
    BattleMoveHandler(const PokemonState& wildState, const std::array<PokemonState, 6>& partyStates);

signals:
    void actionRoundOver(Battler& opponent, Battler& player, bool playerFirst = true, int switchedIn = -1, int shakes = -1);

public slots:
    void startActionRound(int playerMoveIndex, const char* action);

private:
    Battler* createBattler(const PokemonState& state);
    void applyMove(const Move* moveToApply, Battler* caster, Battler* target);

    Battler* m_battleOpponent;
    std::array<Battler*, 6> m_battleParty;
    int m_chosenPartyIndex = 0;
    int m_partyPokemonSentOut[6] = {-1,-1,-1,-1,-1,-1};
    std::mt19937 m_rng;
};

#endif

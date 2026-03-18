#ifndef BATTLEMOVEHANDLER_H
#define BATTLEMOVEHANDLER_H

#include <QObject>
#include <data_move.h>
#include <gamestate.h>
#include <variant>

enum class StatusCondition{
    Burn, Paralyze, Freeze, Sleep, Confuse, Seeded, Cursed
};

struct StatChange{
    int statIndex;
    int amount;
};

enum class WeatherCondition{
    Clear, Sandstorm, Rain, Sunny, Hail
};

using Effect = std::variant<StatusCondition, StatChange>;

struct SideEffect{
    Effect effect;
    int probability;
};

enum class MoveCategory {
    Physical, Special, Status
};

struct Static{
    int uid;
    int stats[6];
    Type types[2];
    int moves[4];
};

struct State{
    int currentHealth;
    StatusCondition conditions[10];
};

struct Battler{
    Static pokeStatic;
    State pokeState;
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

    WeatherCondition m_weatherCondition;

    int calculateDamage();
};

#endif

#ifndef BATTLEMOVEHANDLER_H
#define BATTLEMOVEHANDLER_H
#include <QObject>
#include <data_move.h>
#include <PokemonDatabase.h>
#include <qglobal.h>
#include <qtmetamacros.h>
#include <random>
#include <array>
#include <QVariant>

struct BattleStateDelta{
    bool switchedIn = false;
    int damage = 0;
    int drain = 0;
    int heal = 0;
    std::array<int, 5> deltaStatModifiers = {0, 0, 0, 0, 0};
    std::array<bool, 5> statMaxed = {false, false, false, false, false};
    std::array<bool, 5> statMinned = {false, false, false, false, false};
    bool miss = false;
    bool superEffective = false;
    bool notVeryEffective = false;
    bool noEffect = false;
    bool critical = false;
    bool flinched = false;
    bool sleep = false;
    bool paralyzed = false;
    bool freeze = false;
    int ailmentDamage = -1;
    int confusedDamage = -1;
    Ailment addStatusCondition = Ailment::Null;
    Ailment removeStatusCondition = Ailment::Null;
    bool addConfusion = false;
    bool removeConfusion = false;
    bool failedStatusCondition = false;
    bool failedConfusion = false;
    bool isFirst = false;
};

struct PokeState{
    std::string name;
    int lvl;
    int xpForWinner;
    const Type* types[2];
    const Move* moves[4];
    std::array<int, 6> stats;
};

struct BattleState{
    int currentHealth = -1;
    Ailment statusCondition = Ailment::Null;
    Ailment confused = Ailment::Null;
    int statusConditionCounter = -1;
    int confusedCounter = -1;
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
    ~BattleMoveHandler();
    void switchPartyMember(int newChosenIndex);


signals:
    void actionSequenceReady(QVariantList sequence);

public slots:
    void startActionRound(int playerMoveIndex, QString action);

private:
    int m_chosenIndex = 0;
    Battler* createBattler(const PokemonState& state);
    void applyMove(const Move* _move, Battler* caster, Battler* target);
    void applySecondaryEffects(const Move* _move, Battler* target);
    void applyEndOfTurnEffects(Battler* battler);
    bool canBattlerMove(Battler* caster);
    int calculateTypeEffectiveness(const Move* _move, Battler* target);
    int applyStatModifier(int baseStat, int modifier);

    QVariantList generateActionSequence(Battler& opponent, Battler& player, bool playerFirst, int switchedIn, int shakes);
    void generateMoveSequence(QVariantList& sequence, Battler& attacker, Battler& defender, bool isAttackerPlayer);
    void logActionSequence(const QVariantList& sequence);

    QVariantMap createTextAction(const QString& message, int delay);
    QVariantMap createAttackAction(const QString& role, int delay);
    QVariantMap createDamageAction(const QString& role, int delay);
    QVariantMap createHealthChangeAction(const QString& role, int amount, int delay);
    QVariantMap createCatchAction(int shakes, int delay);
    QVariantMap createStatusCondition(const QString& role, Ailment ailment);
    QVariantMap createEndAction();

    QString ailmentToApplicationText(Ailment ailment);
    QString ailmentToHurtText(Ailment ailment);
    QString ailmentToRemovalText(Ailment ailment);
    void addPostMoveEffects(QVariantList& sequence, Battler& battler, const QString& name, bool isPlayer);
    void addEndOfTurnEffects(QVariantList& sequence, Battler& battler, const QString& name, bool isPlayer);
    QString getStatName(int statIndex);

    Battler* m_battleOpponent;
    std::array<Battler*, 6> m_battleParty;
    int m_partyPokemonSentOut[6] = {-1,-1,-1,-1,-1,-1};
    std::mt19937 m_rng;

    int ms_moveUsedText = 300;
    int ms_ailmentText = 300;
    int ms_statusConditionText = 500;
    int ms_attackAnimation = 500;
    int ms_damageAnimation = 200;
    int ms_healthChange = 1000;
    int ms_criticalHitText = 800;
    int ms_effectivenessText = 800;
    int ms_drainEffectText = 800;
    int ms_catchStart = 1000;
};

#endif

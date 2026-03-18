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
#include <vector>

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
    bool removeStatusCondition = false;
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
    int catchRate;
    const Type* types[2];
    const Move* moves[4];
    std::array<int, 6> stats;
};

struct BattleState{
    int currentHealth = -1;
    Ailment statusCondition = Ailment::Null;
    Ailment confused = Ailment::Null;
    int conditionTurns = -1;
    int confusedTurns = -1;
    int conditionCounter = -1;
    int confusedCounter = -1;
    std::array<int, 5> statModifiers = {0, 0, 0, 0, 0};
    int lastMoveIndex = -1;
};

struct Battler{
    PokeState pokeState;
    BattleState battleState;
    BattleStateDelta delta;
};

struct BattleActionResult {
    enum EffectType {
        DAMAGE,
        STATUS_APPLIED,
        STATUS_REMOVED,
        STAT_CHANGED,
        HEAL,
        DRAIN,
        MISS,
        CRITICAL,
        SUPER_EFFECTIVE,
        NOT_VERY_EFFECTIVE,
        NO_EFFECT,
        TEXT,
        CONFUSION_ADDED,
        CONFUSION_REMOVED,
        CONFUSION_SELF_HIT,
        FLINCH
    };

    struct Effect {
        EffectType type;
        Battler* source = nullptr;
        Battler* target = nullptr;
        int amount = 0;
        Ailment ailment = Ailment::Null;
        int statIndex = -1;
        int statChange = 0;
        std::string text;
    };

    std::vector<Effect> effects;
    bool moveExecuted = false;

    void addEffect(EffectType type, Battler* source = nullptr, Battler* target = nullptr,
                   int amount = 0, Ailment ailment = Ailment::Null,
                   int statIndex = -1, int statChange = 0, const std::string& text = "") {
        effects.push_back({type, source, target, amount, ailment, statIndex, statChange, text});
    }
};

class BattleMoveHandler : public QObject{
    Q_OBJECT
public:
    BattleMoveHandler(const PokemonState& wildState, const std::array<PokemonState, 6>& partyStates);
    ~BattleMoveHandler();
    BattleMoveHandler(const BattleMoveHandler&) = delete;
    BattleMoveHandler& operator=(const BattleMoveHandler&) = delete;
    void switchPartyMember(int newChosenIndex);

signals:
    void actionSequenceReady(QVariantList sequence);

public slots:
    void startActionRound(int playerMoveIndex, QString action);

private:
    int m_chosenIndex = 0;

    Battler* createBattler(const PokemonState& state);
    BattleActionResult applyMove(const Move* _move, Battler* caster, Battler* target);
    BattleActionResult applySecondaryEffects(const Move* _move, Battler* target, bool damageLanded);
    BattleActionResult applyEndOfTurnEffects(Battler* battler);
    BattleActionResult canBattlerMove(Battler* caster);
    void checkRemoveAilment(Battler& battler, BattleActionResult& result);
    void resetDeltaState(BattleStateDelta& delta);

    void applyBattleResult(const BattleActionResult& result);
    QVariantList generateSequenceFromResult(const BattleActionResult& result);

    QVariantList generateActionSequence(Battler& opponent, Battler& player, bool playerFirst, int switchedIn, int shakes);
    void logActionSequence(const QVariantList& sequence);
    void logBattleResult(const BattleActionResult& result);

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

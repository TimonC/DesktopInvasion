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
#include <unordered_set>

struct BattleStateDelta{
    bool switchedIn = false;
    int damage = 0;
    int drain = 0;
    int heal = 0;
    std::array<int, 7> deltaStatModifiers = {0, 0, 0, 0, 0, 0, 0};
    bool miss = false;
    bool superEffective = false;
    bool notVeryEffective = false; bool noEffect = false;
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
    int catchRate;
    int baseXP;
    const Type* types[2];
    const Move* moves[4];
    std::array<int, 6> stats; //HP, Atk, Def, SpAtk, SpDef, Speed
};

struct BattleState{
    int currentHealth = -1;
    Ailment statusCondition = Ailment::Null;
    Ailment confused = Ailment::Null;
    int conditionTurns = -1;
    int confusedTurns = -1;
    int conditionCounter = -1;
    int confusedCounter = -1;
    std::array<int, 7> statModifiers = {0, 0, 0, 0, 0, 0, 0}; //Atk, Def, SpAtk, SpDef, Speed, Accuracy, Evasion
    int lastMoveIndex = -1;
};

struct Battler{
    PokeState pokeState;
    BattleState battleState;
    BattleStateDelta delta;
};

struct BattleActionResult {
    enum EffectType {
        CHANGE_HEALTH,
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
    BattleMoveHandler(const PokemonState& wildState, const std::array<PokemonState, 6>& partyStates, std::mt19937 &rng);
    ~BattleMoveHandler();
    BattleMoveHandler(const BattleMoveHandler&) = delete;
    BattleMoveHandler& operator=(const BattleMoveHandler&) = delete;
    QString switchPartyMember(int newChosenIndex);
    std::array<int, 6> getExperienceSpread();

signals:
    void actionSequenceReady(QVariantList sequence);

public slots:
    void startActionRound(int playerMoveIndex, QString action);

private:
    int m_chosenIndex = 0;
    std::unordered_set<int> m_includedPartyIndices;

    Battler* createBattler(const PokemonState& state);
    BattleActionResult applyMove(const Move* _move, Battler* caster, Battler* target, bool otherHasHadTurn = false);
    BattleActionResult applySecondaryEffects(const Move* _move, Battler* caster, Battler* target, bool damageLanded, bool otherHasHadTurn);
    BattleActionResult applyEndOfTurnEffects(Battler* battler);
    BattleActionResult canBattlerMove(Battler* caster);
    void checkRemoveAilment(Battler& battler, BattleActionResult& result);
    void resetDeltaState(BattleStateDelta& delta);
    void incrementConditionCounters();
    QVariantList processEndOfTurnEffects();

    void applyBattleResult(const BattleActionResult& result);
    QVariantList generateSequenceFromResult(const BattleActionResult& result);

    QVariantList generateActionSequence(Battler& opponent, Battler& player, bool playerFirst, int switchedIn, int shakes);

    QVariantMap createTextAction(const QString& message, int delay);
    QVariantMap createAttackAction(const QString& role, int delay);
    QVariantMap createTakeDamageAction(const QString& role, int delay);
    QVariantMap createChangeHealthAction(const QString& role, int amount, int delay);
    QVariantMap createCatchAction(int shakes, int delay);
    QVariantMap createStatusCondition(const QString& role, Ailment ailment, bool remove);
    QVariantMap createEndAction();

    QString ailmentToApplicationText(Ailment ailment);
    QString ailmentToHurtText(Ailment ailment);
    QString ailmentToRemovalText(Ailment ailment, const QString& pokemonName);
    const QString ailmentToLabel(Ailment ailment);
    QString getStatName(int statIndex);

    Battler* m_battleOpponent;
    std::array<Battler*, 6> m_battleParty;
    int m_partyPokemonSentOut[6] = {-1,-1,-1,-1,-1,-1};
    std::mt19937 &m_rng;
    std::uniform_int_distribution<int> m_moveChoiceDist;
    bool m_expShare = false;
    int m_nrInParty = 0;

    static constexpr int ms_moveUsedText = 800;
    static constexpr int ms_ailmentText = 800;
    static constexpr int ms_statusConditionText = 800;
    static constexpr int ms_attackAnimation = 800;
    static constexpr int ms_healthChange = 1000;
    static constexpr int ms_takeDamage = 400;
    static constexpr int ms_criticalHitText = 800;
    static constexpr int ms_effectivenessText = 800;
    static constexpr int ms_drainEffectText = 800;
    static constexpr int ms_catchStart = 1000;
    static constexpr int ms_ballUsed = 400;
    static constexpr int ms_failCatch = 800;
    static constexpr int ms_successCatch = 800;
};

#endif

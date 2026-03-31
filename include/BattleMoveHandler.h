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
#include <BattleAI.h>

class BattleMoveHandler : public QObject{
    Q_OBJECT
public:
    BattleMoveHandler(const PokemonState& wildState, const std::array<PokemonState, 6>& partyStates, std::string playerName, std::mt19937 &rng);
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
    BattleAI m_battleAI;

    int m_chosenIndex = 0;
    std::unordered_set<int> m_includedPartyIndices;

    QString m_playerName;

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

    QVariantMap createTextAction(const QString& message, int delay, bool isAttackTag = false);
    QVariantMap createAttackAction(const QString& role, int delay);
    QVariantMap createSideToSideAction(const QString& role, int delay);
    QVariantMap createJumpAction(const QString& role, int delay);
    QVariantMap createTakeDamageAction(const QString& role, int delay);
    QVariantMap createChangeHealthAction(const QString& role, int amount, int delay);
    QVariantMap createCatchAction(int shakes, int delay, int ballId);
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

    static constexpr int ms_moveUsedText = 600;
    static constexpr int ms_ailmentText = 600;
    static constexpr int ms_statusConditionText = 600;
    static constexpr int ms_attackAnimation = 600;
    static constexpr int ms_healthChange = 800;
    static constexpr int ms_takeDamage = 400;
    static constexpr int ms_criticalHitText = 600;
    static constexpr int ms_effectivenessText = 600;
    static constexpr int ms_drainEffectText = 600;
    static constexpr int ms_catchStart = 800;
    static constexpr int ms_ballUsed = 400;
    static constexpr int ms_failCatch = 800;
    static constexpr int ms_successCatch = 800;
};

#endif

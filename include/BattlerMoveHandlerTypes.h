#ifndef BATTLEMOVEHANDLERTYPES_H
#define BATTLEMOVEHANDLERTYPES_H

#include <PokeTypes.h>
#include <data_move.h>

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
        CHANGE_HEALTH_END_OF_TURN,
        STATUS_APPLIED,
        STATUS_REMOVED,
        STAT_CHANGED,
        HEAL,
        DRAIN,
        RECOIL,
        MISS,
        CRITICAL,
        SUPER_EFFECTIVE,
        NOT_VERY_EFFECTIVE,
        NO_EFFECT,
        TEXT,
        CONFUSION_ADDED,
        CONFUSION_REMOVED,
        CONFUSED_TURN,
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

#endif

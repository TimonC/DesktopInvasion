#ifndef BATTLEAI_H
#define BATTLEAI_H

#include <random>
#include "data_move.h"
#include <BattlerMoveHandlerTypes.h>

//NOTE: this whole thing was generated with DeepSeek (v3, 26th of March 2026)
//      I just wanna finish the damn game, forgive my lazyness. I figured I'd keep in
//      the AI-style comments!
//
//      What I did is I showed how my current battlemovehandler works,
//      then compared it with the Pokemon Emerald battle AI (default "Trainer" AI, tier 7)
//      and asked it to generate this handler class for me.
//
//      The wonderful folks of Emerald Kaizo have figured out how the AI works,
//      and compiled it in this document which is what I used for reference:
//      https://docs.google.com/document/d/1_FP5FMyyZQkL93jEf6288WtMxOBLPd-Bsemg5HxMbR4/edit?tab=t.0
class BattleAI {
public:
    // Constructor – needs a random number generator for probability checks
    BattleAI(std::mt19937& rng);

    // Returns the index (0..3) of the best move for the opponent to use against the player
    int selectMove(const Battler& opponent, const Battler& player);

private:
    // Main scoring function for a single move
    int evaluateMove(const Move* _move, const Battler& caster, const Battler& target);

    // Estimate damage (const because it doesn't modify AI state)
    int estimateDamage(const Move* _move, const Battler& caster, const Battler& target) const;

    // ----- Core flags implementation (simplified to your requested mechanics) -----

    // Flag 0 – Check Bad Move
    int flag0BadMove(const Move* _move, const Battler& caster, const Battler& target);

    // Flag 1 – Try To Faint
    int flag1TryToFaint(const Move* _move, const Battler& caster, const Battler& target);

    // Flag 2 – Check Viability
    int flag2Viability(const Move* _move, const Battler& caster, const Battler& target);

    // ----- Helper functions -----

    // Check if the move would be immune due to typing (ignoring abilities)
    bool isTypeImmune(const Move* _move, const Battler& target) const;

    // Return effectiveness multiplier (0.0, 0.5, 1.0, 2.0, 4.0)
    double getEffectiveness(const Move* _move, const Battler& target) const;

    // Estimate if the move can faint the target (damage > current HP)
    bool canFaint(const Move* _move, const Battler& caster, const Battler& target) const;

    // Get a random number in [0,255) for probability checks (like the original 256‑based odds)
    int random255() const;

    // Reference to the RNG (used for all probabilistic decisions)
    std::mt19937& m_rng;
};

#endif

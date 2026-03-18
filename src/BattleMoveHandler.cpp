#include "data_move.h"
#include <BattleMoveHandler.h>
#include <globals.h>

BattleMoveHandler::BattleMoveHandler(Battler opponent, Battler party[6])
    : m_opponent(opponent)
{
    std::copy(party, party + 6, m_party);
};

void BattleMoveHandler::startActionRound(int playerMoveIndex){
   int playerFirst = rand()>0.5;
   Battler& player = m_party[m_chosenPartyIndex];


};

void BattleMoveHandler::applyMove(const Move* _move, Battler& caster, Battler& target){
    int statCategoryId=-1;
    int atk = caster.pokeStatic.stats[statCategoryId];
    int def = target.pokeStatic.stats[statCategoryId + 2];

};

/* https://bulbapedia.bulbagarden.net/wiki/Damage */
const int BattleMoveHandler::calculateDamage(const DamageParams& params) {
    float random = 1.0f - (std::rand() % 16) / 100.0f;
    return static_cast<int>(
        (params.power * params.attack / (50.0f * params.defense) *
        (2.0f + 0.4f * params.level) * params.burn + 2.0f) *
        params.critical * random * params.stab *
        params.type1 * params.type2
    );
}

/* #include "Battlescene.h" */
#include "WildPokemon.h"
#include "globals.h"
#include "pokemon_data.h"
#include <Player.h>

Player::Player(QObject* parent) : QObject(parent){
    /* qDebug() << "Player constructor called!"; */
    const PokemonInfo* p1 = Globals::getRandomPokemon();
    const PokemonInfo* p2 = Globals::getRandomPokemon();
    m_party[0] = std::make_unique<PokemonInfo>(*p1);
    m_party[1] = std::make_unique<PokemonInfo>(*p2);
    m_pokemonAvailable = true;
};

void Player::iChooseYou(WildPokemon *opp){
    PokemonInfo* chosen_info = m_party[0].value().get();
    const PokemonInfo* opp_info = opp->info;
    std::unique_ptr<Battle> battle = std::make_unique<Battle>(opp->m_currentDirection, opp->position(), opp_info, chosen_info);
    int direction = opp->m_currentDirection;
    m_pokemonAvailable = false;

    /* auto it = m_party.begin(); */
    /* while(it!=m_party.end()){ */
    /*     if(!it->has_value() || it->value()->m_inABattle){ */
    /*         it++; */
    /*         continue; */
    /*     } */

    /*     if(!chosen){ */
    /*         chosen = it->value().get(); */
    /*         _iChooseYou(opp, chosen); */
    /*     }else{ */
    /*         m_pokemonAvailable = true; */
    /*     } */

    /*     it++; */
    /* } */

    if(chosen_info) return;
    assert(false && "No available Pokemon in party");
    return;
};


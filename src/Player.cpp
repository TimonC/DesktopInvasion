#include <WildPokemon.h>
#include <globals.h>
#include <pokemon_data.h>
#include <Player.h>

Player::Player(QObject* parent) : QObject(parent){
    const PokemonInfo* p1 = Globals::getPokemonInfo(356);
    m_party[0] = p1;
    m_pokemonAvailable = true;
};

void Player::iChooseYou(WildPokemon *opp){
    const PokemonInfo* chosen_info = m_party[0];
    const PokemonInfo* opp_info = opp->info;
    emit startABattle(new Battle(opp, chosen_info));
};

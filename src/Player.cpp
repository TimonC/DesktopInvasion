#include "WildPokemon.h"
#include "globals.h"
#include "pokemon_data.h"
#include "Player.h"

Player::Player(QObject* parent) : QObject(parent){
    const PokemonInfo* p2 = Globals::getPokemonByPokedexId(173);
    /* const PokemonInfo* p2 = Globals::getRandomPokemon(); */
    m_party[0] = std::make_unique<PokemonInfo>(*p2);  // This matches the header
    /* m_party[1] = std::make_unique<PokemonInfo>(*p2); */
    m_pokemonAvailable = true;
};

void Player::iChooseYou(WildPokemon *opp){
    // Access the optional's value correctly
    if (!m_party[0].has_value()) {
        assert(false && "No available Pokemon in party");
        return;
    }

    PokemonInfo* chosen_info = m_party[0].value().get();
    const PokemonInfo* opp_info = opp->info;

    // Store in the member vector, not local variable
    m_activeBattles.push_back(std::make_unique<Battle>(opp->m_currentDirection, opp->position(), opp_info, chosen_info));
    /* opp->hide(); */
    /* m_pokemonAvailable = false; */
};

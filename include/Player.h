#ifndef PLAYER_H
#define PLAYER_H

#include <Battle.h>
#include "WildPokemon.h"
#include "pokemon_data.h"
#include <QObject>

using namespace std;
class Player: public QObject{
    Q_OBJECT

public:
    explicit Player(QObject* parent = nullptr);
    void iChooseYou(WildPokemon* opp);
    bool m_pokemonAvailable = false;

private:
    void _iChooseYou(PokemonInfo* opp, PokemonInfo* chosen);
    array<optional<unique_ptr<PokemonInfo>>, 6> m_party;
    std::vector<std::unique_ptr<Battle>> m_activeBattles;
};

#endif

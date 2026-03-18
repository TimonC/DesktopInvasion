#ifndef PLAYER_H
#define PLAYER_H

#include <Battle.h>
#include <WildPokemon.h>
#include <pokemon_data.h>

class Player: public QObject{
    Q_OBJECT

public:
    Player(QObject* parent = nullptr);
    void iChooseYou(WildPokemon* opp);
    bool m_pokemonAvailable = false;
signals:
    void startABattle(Battle* battle);

private:
    void _iChooseYou(PokemonInfo* opp, PokemonInfo* chosen);
    std::array<const PokemonInfo*, 6> m_party;
};

#endif

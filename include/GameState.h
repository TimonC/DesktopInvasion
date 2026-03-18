#ifndef GAMESTATE_H
#define GAMESTATE_H
#include <QPoint>
#include <array>

struct GameState{
    int gameId;
    std::array<int, 6> party;
    struct WildPokemonState;
};

struct WildPokemonState{
    int direction;
    QPoint* position;
    int pokedexId;
};

#endif

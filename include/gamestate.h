#ifndef GAMESTATE_H
#define GAMESTATE_H
#include <string>
#include <PokemonTypes.h>

struct PokemonState {
    int _id = -1;
    int pokedex_id;
    int variant_id = 0;
    int pokeball_id = 0;
    std::string name;

    int lvl = 1;
    int currentXP = 0;

    int ivs[6];
    int evs[6];
    Nature nature;

    int moves[4] = {-1, -1, -1, -1};
};

struct GameState {
    int _id = 1;
    int player_sprite_id = 0;
    std::string name = "Player";
    int party_id[6] = {0, 0, 0, 0, 0, 0};
};

#endif

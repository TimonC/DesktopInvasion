#ifndef GLOBALS_H
#define GLOBALS_H

#include <QRect>
#include "Player.h"
#include <QGuiApplication>

static bool DEBUG = true;
Player& getPlayer();
QRect screenSize();
const PokemonInfo* getRandomPokemon();
const PokemonInfo* findPokemonByPokedexId(int pokedexId);
#endif

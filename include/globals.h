#ifndef GLOBALS_H
#define GLOBALS_H

#include <QRect>
#include "Player.h"
#include <QGuiApplication>

Player& getPlayer();
QRect screenSize();
static bool DEBUG = false;
const PokemonInfo* getRandomPokemon();
const PokemonInfo* findPokemonByPokedexId(int pokedexId);
#endif

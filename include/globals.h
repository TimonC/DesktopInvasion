#ifndef GLOBALS_H
#define GLOBALS_H

#include <QRect>
#include "Player.h"
#include <QGuiApplication>

extern bool DEBUG;

const QRect& screenGeometry();

Player& getPlayer();
const PokemonInfo* getRandomPokemon();
const PokemonInfo* findPokemonByPokedexId(int pokedexId);

#endif

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QRect>
#include "Player.h"
#include <QGuiApplication>

Player& getPlayer();
int screenWidth();
int screenHeight();
static bool DEBUG = false;

#endif

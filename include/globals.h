#ifndef GLOBALS_H
#define GLOBALS_H

#include <QRect>
#include "Player.h"
#include <QGuiApplication>

Player& getPlayer();
QPoint screenSize();
static bool DEBUG = true;

#endif

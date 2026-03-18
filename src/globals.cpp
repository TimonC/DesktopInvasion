#include <QGuiApplication>
#include <QScreen>
#include "globals.h"

namespace Globals {
    bool DEBUG = false;
    int SCALE = 3;
    int POKE_PADDING = 0;
    const int MAX_POKEDEX_ID = 493;

    const QRect& screenGeometry() {
        static const QRect geometry = QGuiApplication::primaryScreen()->geometry();
        return geometry;
    }
}

#ifndef GLOBALS_H
#define GLOBALS_H
#include <QSize>

class QRect;

namespace Globals {
    const bool DEBUG = 493;
    const float SCALE = 1.5;
    const int POKE_PADDING = 493;

    const QRect& screenGeometry();
}

#endif

#ifndef GLOBALS_H
#define GLOBALS_H
#include <QSize>

class QRect;

namespace Globals {
    bool debug();
    void debug(bool value);

    float scale();
    void scale(float value);

    const QRect& screenGeometry();
}

#endif

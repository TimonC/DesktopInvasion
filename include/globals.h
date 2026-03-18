#ifndef GLOBALS_H
#define GLOBALS_H
#include <QSize>

class QRect;

namespace Globals {
    bool debug();
    void debug(bool value);

    float scale();
    void scale(float value);

    float animationSpeed();
    void animationSpeed(float value);

    int encounterLvlLow();
    void encounterLvlLow(float value);

    int encounterLvlHigh();
    void encounterLvlHigh(float value);

    const QRect& screenGeometry();

}

#endif

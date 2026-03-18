#include <QGuiApplication>
#include <QScreen>
#include "globals.h"

namespace Globals {
    const QRect& screenGeometry() {
        static const QRect geometry = QGuiApplication::primaryScreen()->geometry();
        return geometry;
    }
}

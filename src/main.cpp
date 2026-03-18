// main.cpp
#include <QGuiApplication>
#include "InvasionOverlay.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    InvasionOverlay overlay;
    overlay.show();
    // Add some test sprites
    /* overlay.addSprite("qrc:/sprites/Pikachu.qml", 100, 100); */
    /* overlay.addSprite("qrc:/sprites/Charmander.qml", 200, 200); */
    /* overlay.addSprite("qrc:/sprites/Bulbasaur.qml", 300, 300); */

    return app.exec();
}

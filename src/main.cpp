#include <QGuiApplication>
#include "InvasionOverlay.h"
#include <QQuickItem>
#include <QDebug>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    InvasionOverlay overlay;
    overlay.show();

    // Add the sprite
    overlay.addSprite("qrc:/sprites/PokemonSprite.qml", 200, 200);

    // Grab the last added sprite and configure it
    QQuickItem *container = overlay.rootObject()->findChild<QQuickItem*>("spriteContainer");
    if (!container) {
        qDebug() << "spriteContainer not found!";
        return app.exec();
    }

    QList<QQuickItem*> sprites = container->childItems();
    if (!sprites.isEmpty()) {
        QQuickItem *sprite = sprites.last();
        sprite->setProperty("spriteSheet", "qrc:/assets/HGSS/PokGen1_transparent_reordered.png");
        sprite->setProperty("rowIndex", 5);          // choose Pokémon row
        sprite->setProperty("currentDirection", "left");
    } else {
        qDebug() << "No sprite found inside container!";
    }

    return app.exec();
}


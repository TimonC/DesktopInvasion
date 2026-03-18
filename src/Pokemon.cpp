#include <QRandomGenerator>
#include <QGuiApplication>
#include <QQuickItem>
#include <qnamespace.h>
#include "Pokemon.h"
#include <globals.h>

Pokemon::Pokemon(QWindow *parent, int row)
    : QQuickView(parent)
    , m_row(row)
    , m_currentDirection(0)
    , m_scaleFactor(3)
{
    setFlags( Qt::WindowStaysOnTopHint
            | Qt::Tool
            | Qt::WindowDoesNotAcceptFocus
            | Qt::WindowTransparentForInput
            | Qt::FramelessWindowHint);
    setColor(Qt::transparent);

    setSource(QUrl("qrc:/sprites/PokemonSprite.qml"));
    m_sprite = rootObject();
    m_sprite->setProperty("scaleFactor", m_scaleFactor);
    m_sprite->setProperty("row", m_row);
    m_sprite->setProperty("debugLines", DEBUG);

    // Set random spritesheet here
    QString randomSpriteSheet = getRandomSpriteSheet();
    m_sprite->setProperty("spriteSheet", randomSpriteSheet);

    setSize(50);
}

QString Pokemon::getRandomSpriteSheet() {
    int randomValue = std::rand() % 3;
    switch(randomValue) {
        case 0:
            return "qrc:/assets/HGSS/PokGen1_transparent_reordered.png";
        case 1:
            return "qrc:/assets/HGSS/PokGen3_transparent_reordered.png";
        case 2:
            return "qrc:/assets/HGSS/PokGen4_transparent_reordered.png";
        default:
            return "qrc:/assets/HGSS/PokGen1_transparent_reordered.png";
    }
}

void Pokemon::setSize(int size){
    int width = m_scaleFactor*size;
    int height = m_scaleFactor*size;

    m_sprite->setProperty("itemWidth", width);
    m_sprite->setProperty("itemHeight", height);

    setWidth(width);
    setHeight(height);

    m_sprite->setProperty("spriteOffsetX", width/2.5);
    m_sprite->setProperty("spriteOffsetY", height/2.8);
}

void Pokemon::useMove(){
    m_sprite->setProperty("tackle", true);
}

void Pokemon::attacked(){
    m_sprite->setProperty("attacked", true);
}

void Pokemon::direction(int direction){
    m_currentDirection = direction%4;
    m_sprite->setProperty("direction",m_currentDirection);
};

int Pokemon::direction(){
    return m_currentDirection;
};

QPoint Pokemon::movePos(QPoint delta, bool boundsCheck) {
    QPoint pos = position();
    /* if(boundsCheck){ */
        /* if (pos.x() + delta.x() < 0 || pos.x() + delta.x() > getScreenGeometry().width() - SPRITE_SIZE) */
            /* delta.setX(0); */
        /* if (pos.y() + delta.y() < 0 || pos.y() + delta.y() > getScreenGeometry().height() - SPRITE_SIZE) */
            /* delta.setY(0); */
    /* } */
    QPoint newPos = pos + delta;
    setPosition(newPos);
    return newPos;
}

#include <QRandomGenerator>
#include <QGuiApplication>
#include <QQuickItem>
#include "globals.h"
#include "Pokemon.h"

Pokemon::Pokemon(QWindow *parent, int row)
    : QQuickView(parent)
    , m_row(row)
    , m_currentDirection(0)
    , m_scaleFactor(3)
{
    setFlags( Qt::WindowStaysOnTopHint
            | Qt::Tool
            | Qt::WindowDoesNotAcceptFocus
            | Qt::FramelessWindowHint);
    setColor(Qt::transparent);


    setSource(QUrl("qrc:/sprites/PokemonSprite.qml"));
    m_sprite = rootObject();
    m_sprite->setProperty("scaleFactor", m_scaleFactor);
    m_sprite->setProperty("row", m_row);

    int width = m_scaleFactor*32;
    int height = m_scaleFactor*32;
    m_sprite->setProperty("itemWidth", width);
    m_sprite->setProperty("itemHeight", height);
    setWidth(width);
    setHeight(height);

    m_sprite->setProperty("spriteOffsetX", 32);
    m_sprite->setProperty("spriteOffsetY",32/2);

    QRect& screen = getScreenGeometry();
    setX(std::rand()%screen.width());
    setY(std::rand()%screen.height());
}


void Pokemon::direction(int direction){
    m_currentDirection = direction%4;
    m_sprite->setProperty("animation",m_currentDirection);
};
int Pokemon::direction(){
    return m_currentDirection;
};


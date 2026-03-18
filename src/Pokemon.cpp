#include <QRandomGenerator>
#include <QGuiApplication>
#include <QQuickItem>
#include <qnamespace.h>
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
            | Qt::WindowTransparentForInput
            | Qt::FramelessWindowHint);
    setColor(Qt::transparent);


    setSource(QUrl("qrc:/sprites/PokemonSprite.qml"));
    m_sprite = rootObject();
    m_sprite->setProperty("scaleFactor", m_scaleFactor);
    m_sprite->setProperty("row", m_row);

    setSize(50);


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


#include "Pokemon.h"
#include <QRandomGenerator>
#include <QGuiApplication>
#include <QScreen>
#include <QQuickItem>

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

    m_screenGeometry = QGuiApplication::primaryScreen()->geometry();
    setX(std::rand()%m_screenGeometry.width());
    setY(std::rand()%m_screenGeometry.height());
}






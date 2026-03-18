#include "PokemonInteractable.h"
#include <QRandomGenerator>
#include <QGuiApplication>
#include <QScreen>
#include <QQuickItem>
#include <QTimer>

PokemonInteractable::PokemonInteractable(QWindow *parent)
    : QQuickView(parent)
    , m_decisionTimer(new QTimer(this))
    , m_moveTimer(new QTimer(this))
    , m_moveSpeed(2 + QRandomGenerator::global()->bounded(3))
    , m_currentDirection(0)
{
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setColor(Qt::transparent);
    setResizeMode(SizeViewToRootObject);

    setSource(QUrl("qrc:/sprites/PokemonSprite.qml"));
    m_wildPokemon = rootObject();

    setWidth(SPRITE_SIZE);
    setHeight(SPRITE_SIZE);

    m_screenGeometry = QGuiApplication::primaryScreen()->geometry();
    setX((m_screenGeometry.width() - SPRITE_SIZE) / 2);
    setY((m_screenGeometry.height() - SPRITE_SIZE) / 2);

    m_decisionTimer->setInterval(1000 + QRandomGenerator::global()->bounded(2000));
    connect(m_decisionTimer, &QTimer::timeout, this, &PokemonInteractable::makeRandomDecision);

    m_moveTimer->setInterval(50); // 20fps
    connect(m_moveTimer, &QTimer::timeout, this, &PokemonInteractable::moveStep);

    m_decisionTimer->start();
    makeRandomDecision();
}

void PokemonInteractable::makeRandomDecision(){
    int decision = QRandomGenerator::global()->bounded(8);

    m_currentDirection = decision / 2;
    m_wildPokemon->setProperty("animation", m_currentDirection);

    bool moving = (decision % 2) == 1;
    if (moving) {
        m_moveTimer->start();
    } else {
        m_moveTimer->stop();
    }
}

void PokemonInteractable::moveStep(){
    int newX = x();
    int newY = y();

    if (m_currentDirection == 0) {
        newY -= m_moveSpeed;
    } else if (m_currentDirection == 1) {
        newX -= m_moveSpeed;
    } else if (m_currentDirection == 2) {
        newY += m_moveSpeed;
    } else if (m_currentDirection == 3) {
        newX += m_moveSpeed;
    }

    newX = qMax(0, qMin(newX, m_screenGeometry.width() - SPRITE_SIZE));
    newY = qMax(0, qMin(newY, m_screenGeometry.height() - SPRITE_SIZE));

    setX(newX);
    setY(newY);

    if (newX == 0 || newX == m_screenGeometry.width() - SPRITE_SIZE ||
        newY == 0 || newY == m_screenGeometry.height() - SPRITE_SIZE) {
        makeRandomDecision();
    }
}

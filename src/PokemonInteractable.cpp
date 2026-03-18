#include "PokemonInteractable.h"
#include <QRandomGenerator>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <QQuickItem>
#include <QQuickView>
#include <qrandom.h>
#include <qtimer.h>
PokemonInteractable::PokemonInteractable(QWindow *parent, int row)
    : QQuickView(parent)
    , m_row(row)
    , m_decisionTimer(new QTimer(this))
    , m_moveTimer(new QTimer(this))
    , m_openingTimer(new QTimer(this))
    , m_moveSpeed(1 + QRandomGenerator::global()->bounded(2))
    , m_currentDirection(0)
    , m_scaleFactor(3)
{
    setFlags( Qt::WindowStaysOnTopHint
            | Qt::Tool
            | Qt::WindowDoesNotAcceptFocus
            | Qt::FramelessWindowHint);

    setColor(Qt::transparent);


    setSource(QUrl("qrc:/sprites/PokemonSprite.qml"));
    m_wildPokemon = rootObject();
    m_wildPokemon->setProperty("scaleFactor", m_scaleFactor);
    m_wildPokemon->setProperty("row", m_row);

    int width = 4*32;
    int height = 4*32;
    m_wildPokemon->setProperty("itemWidth", width);
    m_wildPokemon->setProperty("itemHeight", height);

    m_wildPokemon->setProperty("spriteOffsetX", 32);
    m_wildPokemon->setProperty("spriteOffsetY",32);

    QQuickItem* mouseArea = m_wildPokemon->property("mouseArea").value<QQuickItem*>();
    connect(mouseArea, SIGNAL(clicked(QQuickMouseEvent*)), this, SLOT(onClick()));

    m_screenGeometry = QGuiApplication::primaryScreen()->geometry();
    setX((m_screenGeometry.width() - SPRITE_SIZE) / 2);
    setY((m_screenGeometry.height() - SPRITE_SIZE) / 2);

    m_decisionTimer->setInterval(1000 + QRandomGenerator::global()->bounded(2000));
    connect(m_decisionTimer, &QTimer::timeout, this, &PokemonInteractable::makeRandomDecision);

    m_moveTimer->setInterval(50); // 20fps
    connect(m_moveTimer, &QTimer::timeout, this, &PokemonInteractable::moveStep);

    m_openingTimer->setInterval(0);
    connect(m_openingTimer, &QTimer::timeout, this, &PokemonInteractable::stopOpening);

    m_decisionTimer->start();
    makeRandomDecision();
}



void PokemonInteractable::onClick(){
    m_moveTimer->stop();
    m_decisionTimer->stop();

    m_wildPokemon->setProperty("jumping", true);
    startOpening();
}

void PokemonInteractable::startOpening(int durationMs){
    m_wildPokemon->setProperty("openingButtons", true);
    m_openingTimer->start(durationMs);
}

void PokemonInteractable::stopOpening(){
    m_wildPokemon->setProperty("openingButtons", false);
    m_openingTimer->stop();
    makeRandomDecision();
    m_decisionTimer->start();
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



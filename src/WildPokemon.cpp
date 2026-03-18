#include "Pokemon.h"
#include "WildPokemon.h"
#include <QWindow>
#include <QTimer>
#include <QQuickItem>
#include <QQuickView>
#include <QRandomGenerator>

WildPokemon::WildPokemon(QWindow *parent, int row)
    : Pokemon(parent, row)
    , m_decisionTimer(new QTimer(this))
    , m_moveTimer(new QTimer(this))
    , m_openingTimer(new QTimer(this))
    , m_moveSpeed(1 + QRandomGenerator::global()->bounded(2))
{
    QQuickItem* mouseArea = m_sprite->property("mouseArea").value<QQuickItem*>();
    connect(mouseArea, SIGNAL(clicked(QQuickMouseEvent*)), this, SLOT(onClick()));

    m_decisionTimer->setInterval(1000 + QRandomGenerator::global()->bounded(2000));
    connect(m_decisionTimer, &QTimer::timeout, this, &WildPokemon::makeRandomDecision);

    m_moveTimer->setInterval(50); // 20fps
    connect(m_moveTimer, &QTimer::timeout, this, &WildPokemon::moveStep);

    connect(m_openingTimer, &QTimer::timeout, this, &WildPokemon::stopOpening);

    QQuickItem* openingButtons = m_sprite->property("battleButton").value<QQuickItem*>();
    connect( openingButtons, SIGNAL(clicked()), this, SLOT(startBattle()));

    m_decisionTimer->start();
    makeRandomDecision();

}

void WildPokemon::startBattle(){
    QQuickItem* mouseArea = m_sprite->property("mouseArea").value<QQuickItem*>();
    disconnect(mouseArea, SIGNAL(clicked(QQuickMouseEvent*)), this, SLOT(onClick()));

    mouseArea->property("enabled")=false  ;
    mouseArea->property("visible")=false  ;

    stopOpening();
    m_moveTimer->disconnect();
    m_decisionTimer->disconnect();

    qDebug() << "Battle started!";
}


void WildPokemon::onClick(){
    m_moveTimer->stop();
    m_decisionTimer->stop();

    m_sprite->setProperty("jumping", true);
    startOpening();
}

void WildPokemon::startOpening(int durationMs){
    m_sprite->setProperty("openingButtons", true);
    m_openingTimer->start(durationMs);
}

void WildPokemon::stopOpening(){
    m_sprite->setProperty("openingButtons", false);
    m_openingTimer->stop();
    m_decisionTimer->start();
}

void WildPokemon::makeRandomDecision(){
    int decision = QRandomGenerator::global()->bounded(8);

    m_currentDirection = decision / 2;
    m_sprite->setProperty("animation", m_currentDirection);
bool moving = (decision % 2) == 1; if (moving) {
        m_moveTimer->start();
    } else {
        m_moveTimer->stop();
    }
}

void WildPokemon::moveStep(){
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

#include <QWindow>
#include <QTimer>
#include <QQuickItem>
#include <QQuickView>
#include <QRandomGenerator>
#include "Pokemon.h"
#include "WildPokemon.h"
#include "Player.h"
#include "globals.h"

WildPokemon::WildPokemon(QWindow *parent, int row)
    : Pokemon(parent, row)
    , m_decisionTimer(new QTimer(this))
    , m_moveTimer(new QTimer(this))
    , m_moveSpeed(1 + QRandomGenerator::global()->bounded(2))
{
    QQuickItem* mouseArea = m_sprite->property("mouseArea").value<QQuickItem*>();
    connect(mouseArea, SIGNAL(clicked(QQuickMouseEvent*)), this, SLOT(onClick()));

    m_decisionTimer->setInterval(1000 + QRandomGenerator::global()->bounded(2000));
    connect(m_decisionTimer, &QTimer::timeout, this, &WildPokemon::makeRandomDecision);

    m_moveTimer->setInterval(50); // 20fps
    connect(m_moveTimer, &QTimer::timeout, this, &WildPokemon::moveStep);

    QQuickItem* openingButtons = m_sprite->property("battleButton").value<QQuickItem*>();
    connect( openingButtons, SIGNAL(clicked()), this, SLOT(startBattle()));

    QRect& screen = getScreenGeometry();
    setX(screen.width()/2 + ((std::rand()%2)*2-1) * std::rand()%screen.width()/2);
    setY(screen.height()/2 + ((std::rand()%2)*2-1) * std::rand()%screen.height()/2);
    m_decisionTimer->start();
    makeRandomDecision();

}

void WildPokemon::startBattle(){
    getPlayer().iChooseYou(this);

    QQuickItem* mouseArea = m_sprite->property("mouseArea").value<QQuickItem*>();
    disconnect(mouseArea, SIGNAL(clicked(QQuickMouseEvent*)), this, SLOT(onClick()));

    mouseArea->setProperty("enabled", false);
    mouseArea->setProperty("visible", false);
    m_sprite->setProperty("openingButtons", false);

    m_moveTimer->disconnect();
    m_decisionTimer->disconnect();
}


void WildPokemon::onClick(){
    m_moveTimer->stop();
    m_decisionTimer->stop();

    m_sprite->setProperty("jumping", true);
    startOpening();
}

void WildPokemon::startOpening(int durationMs){
    if(getPlayer().pokemonAvailable()){
        m_sprite->setProperty("openingButtons", true);
    }else{
        qDebug() << "No party Pokemon available for battle.";
    }
    QTimer::singleShot(durationMs, this, &WildPokemon::stopOpening);
}

void WildPokemon::stopOpening(){
    m_sprite->setProperty("openingButtons", false);
    m_decisionTimer->start();
}

void WildPokemon::makeRandomDecision(){
    int decision = QRandomGenerator::global()->bounded(8);
    direction(decision/2);

    bool moving = (decision % 2) == 1;
    if (moving) {
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

    QRect& screen = getScreenGeometry();
    newX = qMax(0, qMin(newX, screen.width() - SPRITE_SIZE));
    newY = qMax(0, qMin(newY, screen.height() - SPRITE_SIZE));

    setX(newX);
    setY(newY);

    if (newX == 0 || newX == screen.width() - SPRITE_SIZE ||
        newY == 0 || newY == screen.height() - SPRITE_SIZE) {
        makeRandomDecision();
    }
}

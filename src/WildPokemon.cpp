#include <QWindow>
#include <QTimer>
#include <QQuickItem>
#include <QQuickView>
#include <QRandomGenerator>
#include <qnamespace.h>
#include "Pokemon.h"
#include "WildPokemon.h"
#include "Player.h"
#include "globals.h"

WildPokemon::WildPokemon(QWindow *parent, int row)
    : Pokemon(parent, row)
    , m_hitbox(new Hitbox(nullptr))
    , m_decisionTimer(new QTimer(this))
    , m_moveTimer(new QTimer(this))
    , m_moveSpeed(1 + QRandomGenerator::global()->bounded(2))
{
    QRect& screen = getScreenGeometry();
    setX(screen.width()/2);//+ ((std::rand()%2)*2-1) * std::rand()%screen.width()/2);
    setY(screen.height()/2);// + ((std::rand()%2)*2-1) * std::rand()%screen.height()/2);

    m_hitbox->offsetX = width()/5;
    m_hitbox->offsetY = height()/4;
    m_hitbox->setX(x() + m_hitbox->offsetX);
    m_hitbox->setY(y() + m_hitbox->offsetY);
    connect(m_hitbox->m_mouseArea, SIGNAL(doubleClicked(QQuickMouseEvent*)), this, SLOT(handleDoubleClick()));
    connect(m_hitbox->m_battleButton, SIGNAL(clicked()), this, SLOT(startBattle()));

    m_decisionTimer->setInterval(1000 + QRandomGenerator::global()->bounded(2000));
    m_moveTimer->setInterval(50); // 20fps

    startRoaming();
    show();
    m_hitbox->show();
}



void WildPokemon::startRoaming(){
    connect(m_decisionTimer, &QTimer::timeout, this, &WildPokemon::makeRandomDecision);
    connect(m_moveTimer, &QTimer::timeout, this, &WildPokemon::moveStep);

    m_decisionTimer->start();
    makeRandomDecision();
}

void WildPokemon::handleDoubleClick(){
    m_moveTimer->stop();
    m_decisionTimer->stop();

    m_sprite->setProperty("jumping", true);

    if(getPlayer().m_pokemonAvailable)
        m_hitbox->showButton();

    QTimer::singleShot(5000, this, [this]() {
        m_decisionTimer->start();
        m_hitbox->showButton(false);
    });
}

void WildPokemon::startBattle(){
    m_hitbox->showButton(false);

    getPlayer().iChooseYou(this);

    m_moveTimer->disconnect();
    m_decisionTimer->disconnect();
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

    m_hitbox->setX(newX + m_hitbox->offsetX);
    m_hitbox->setY(newY + m_hitbox->offsetY);

    if (newX == 0 || newX == screen.width() - SPRITE_SIZE ||
        newY == 0 || newY == screen.height() - SPRITE_SIZE) {
        makeRandomDecision();
    }
}

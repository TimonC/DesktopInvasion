#include <QWindow>
#include <QTimer>
#include <QQuickItem>
#include <QQuickView>
#include <QRandomGenerator>
#include <qnamespace.h>
#include "Hitbox.h"
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

    m_hitbox->offset =QPoint(width()/3.3, height()/2.8);
    movePos(QPoint(screenSize().width()/2, screenSize().height()/2));

    connect(m_hitbox, &Hitbox::drag, this, [this](QPoint delta){
            movePos(delta);
            });
    connect(m_hitbox, &Hitbox::isDragged, this, &WildPokemon::handleDrag);
    connect(m_hitbox->m_mouseArea, SIGNAL(doubleClicked(QQuickMouseEvent*)), this, SLOT(handleDoubleClick()));
    connect(m_hitbox->m_battleButton, SIGNAL(clicked()), this, SLOT(startBattle()));

    m_decisionTimer->setInterval(1000 + QRandomGenerator::global()->bounded(2000));
    m_moveTimer->setInterval(50); // 20fps

    startRoaming();
    show();
}



void WildPokemon::startRoaming(){
    connect(m_decisionTimer, &QTimer::timeout, this, &WildPokemon::makeRandomDecision);
    connect(m_moveTimer, &QTimer::timeout, this, &WildPokemon::moveStep);

    m_hitbox->show();
    m_decisionTimer->start();
    makeRandomDecision();
}

void WildPokemon::handleDrag(bool isDragged){
    if(isDragged){
        m_sprite->setProperty("frameRate", 8);
        m_moveTimer->stop();
        m_decisionTimer->stop();
    }else{
        m_sprite->setProperty("frameRate", 4);
        m_decisionTimer->start();
    }
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
    m_hitbox->hide();

    switch(m_currentDirection){
        case 0:
            if(y()<50) m_currentDirection=2;
            break;
        case 1:
            if(x()<50) m_currentDirection=3;
            break;
        case 2:
            if(y() + height() > screenSize().y() - 50) m_currentDirection = 0;
            break;
        case 3:
            if(x() + width() > screenSize().x() - 50) m_currentDirection = 1;
            break;
    }

    direction(m_currentDirection);
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

QPoint WildPokemon::movePos(QPoint delta) {
    QPoint newPos = Pokemon::movePos(delta);
    m_hitbox->setPosition(QPoint(newPos.x(), newPos.y()) + m_hitbox->offset);
    return newPos;
}

void WildPokemon::moveStep(){
    QPoint newPos;

    switch (m_currentDirection) {
        case 0: newPos = movePos(QPoint(0, -m_moveSpeed)); break;
        case 1: newPos = movePos(QPoint(-m_moveSpeed, 0)); break;
        case 2: newPos = movePos(QPoint(0, m_moveSpeed)); break;
        case 3: newPos = movePos(QPoint(m_moveSpeed, 0)); break;
    }
}

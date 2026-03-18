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

    m_hitbox->offset =QPoint(width()/5, height()/4);
    move(QPoint(getScreenGeometry().width()/2, getScreenGeometry().height()/2));

    connect(m_hitbox, &Hitbox::dragged, this, &WildPokemon::move);
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

QPoint WildPokemon::move(QPoint deltas) {
    QPoint pos = position() + deltas;
    int constrainedX = qMax(0, qMin(pos.x(), getScreenGeometry().width() - SPRITE_SIZE));
    int constrainedY = qMax(0, qMin(pos.y(), getScreenGeometry().height() - SPRITE_SIZE));
    QPoint newPos(constrainedX, constrainedY);

    setPosition(newPos);
    m_hitbox->setPosition(QPoint(newPos.x(), newPos.y()) + m_hitbox->offset);

    return newPos;
}

void WildPokemon::moveStep(){
    QPoint newPos;

    switch (m_currentDirection) {
        case 0: newPos = move(QPoint(0, -m_moveSpeed)); break;
        case 1: newPos = move(QPoint(-m_moveSpeed, 0)); break;
        case 2: newPos = move(QPoint(0, m_moveSpeed)); break;
        case 3: newPos = move(QPoint(m_moveSpeed, 0)); break;
    }

    if (newPos.x() == 0 || newPos.x() == getScreenGeometry().width() - SPRITE_SIZE ||
        newPos.y() == 0 || newPos.y() == getScreenGeometry().height() - SPRITE_SIZE) {
        makeRandomDecision();
    }
}

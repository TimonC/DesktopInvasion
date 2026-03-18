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
    , m_hitbox(new QQuickView(nullptr))
    , m_decisionTimer(new QTimer(this))
    , m_moveTimer(new QTimer(this))
    , m_moveSpeed(1 + QRandomGenerator::global()->bounded(2))
{
    setupHitbox();

    QRect& screen = getScreenGeometry();
    setX(screen.width()/2);//+ ((std::rand()%2)*2-1) * std::rand()%screen.width()/2);
    setY(screen.height()/2);// + ((std::rand()%2)*2-1) * std::rand()%screen.height()/2);

    m_decisionTimer->setInterval(1000 + QRandomGenerator::global()->bounded(2000));
    m_moveTimer->setInterval(50); // 20fps
    startRoaming();

    show();
    m_hitbox->show();
}

void WildPokemon::setupHitbox(){
    m_hitbox->setSource(QUrl("qrc:/sprites/Hitbox.qml"));
    m_hitbox->setFlags(
              Qt::WindowStaysOnTopHint
            | Qt::Tool
            | Qt::WindowDoesNotAcceptFocus
            | Qt::FramelessWindowHint);
    m_hitbox->setColor(Qt::transparent);
    QQuickItem* hitbox_sprite = m_hitbox->rootObject();

    QQuickItem* mouseArea = hitbox_sprite->property("mouseArea").value<QQuickItem*>();
    connect(mouseArea, SIGNAL(doubleClicked(QQuickMouseEvent*)), this, SLOT(onSelect()));
    connect(mouseArea, SIGNAL(pressed(QQuickMouseEvent*)), this, SLOT(systemMove()));


    QQuickItem* openingButtons = hitbox_sprite->property("battleButton").value<QQuickItem*>();
    connect( openingButtons, SIGNAL(clicked()), this, SLOT(startBattle()));
}

void WildPokemon::systemMove(){
    m_hitbox->startSystemMove();
}

void WildPokemon::startRoaming(){
    connect(m_decisionTimer, &QTimer::timeout, this, &WildPokemon::makeRandomDecision);
    connect(m_moveTimer, &QTimer::timeout, this, &WildPokemon::moveStep);

    m_decisionTimer->start();
    makeRandomDecision();
}



void WildPokemon::startBattle(){
    m_sprite->setProperty("openingButtons", false);

    getPlayer().iChooseYou(this);

    m_moveTimer->disconnect();
    m_decisionTimer->disconnect();
}


void WildPokemon::onSelect(){
    m_moveTimer->stop();
    m_decisionTimer->stop();

    m_sprite->setProperty("jumping", true);
    startOpening();
}

void WildPokemon::startOpening(int durationMs){
    if(getPlayer().m_pokemonAvailable){
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

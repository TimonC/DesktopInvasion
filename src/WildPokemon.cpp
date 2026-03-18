#include <QWindow>
#include <QTimer>
#include <QQuickItem>
#include <QQuickView>
#include <QRandomGenerator>
#include <qevent.h>
#include <qnamespace.h>
#include "WildPokemon.h"
#include "DesktopScene.h"
#include "globals.h"
#include "pokemon_data.h"

WildPokemon::WildPokemon(const PokemonInfo* info, QWindow *parent)
    : DesktopScene(parent)
    , info(info)
    , m_decisionTimer(new QTimer(this))
    , m_moveTimer(new QTimer(this))
    , m_moveSpeed(1 + QRandomGenerator::global()->bounded(2))
{

    const QRect& screen = Globals::screenGeometry();
    setPosition(QPoint(screen.width()/2, screen.height()/2));

    setSource(QUrl("qrc:/qml/PokemonSprite.qml"));
    m_sprite = rootObject();
    m_sprite->setProperty("spriteSheet", QString("qrc:/assets/HGSS/PokGen%1_transparent_reordered.png").arg(info->generation));
    m_sprite->setProperty("scaleFactor", Globals::SCALE);
    m_sprite->setProperty("row", info->spriteId);
    m_sprite->setProperty("debugLines", Globals::DEBUG);

    const SpriteInfo* spriteInfo = Globals::getSpriteInfo(info->spriteId, info->generation);

    int width = Globals::SCALE* (spriteInfo->max_width + Globals::POKE_PADDING);
    int height = Globals::SCALE * (spriteInfo->max_height + Globals::POKE_PADDING);

    m_sprite->setProperty("itemWidth", width);
    m_sprite->setProperty("itemHeight", height);

    setWidth(width);
    setHeight(height);

    m_sprite->setProperty("spriteOffsetX" , Globals::POKE_PADDING/2);
    m_sprite->setProperty("spriteOffsetY", Globals::POKE_PADDING/2);
    /* if (auto mouseArea = m_sprite->property("mouseArea").value<QQuickItem*>()) */
        /* connect(mouseArea, SIGNAL(doubleClicked(QQuickMouseEvent*)), this, SLOT(handleDoubleClick())); */

    m_decisionTimer->setInterval(1000 + QRandomGenerator::global()->bounded(2000));
    m_moveTimer->setInterval(50); // 20fps


    qDebug() << "A wild" << info->name << "(#" <<info->pokedexId << ") appeared!"
             << "width:" << spriteInfo->max_width << "height:" << spriteInfo->max_height ;

    startRoaming();
    show();
}


void WildPokemon::startRoaming(){
    m_isDragged = false;
    m_sprite->setProperty("frameRate", 4);
    connect(m_decisionTimer, &QTimer::timeout, this, &WildPokemon::makeRandomDecision);
    connect(m_moveTimer, &QTimer::timeout, this, &WildPokemon::moveStep);

    m_decisionTimer->start();
}

void WildPokemon::mouseDoubleClickEvent(QMouseEvent* event){
    startBattle();
};
void WildPokemon::handleDrag(bool isDragged){
    m_isDragged = isDragged;
    if(isDragged){
        m_sprite->setProperty("frameRate", 8);
        m_moveTimer->stop();
        m_decisionTimer->stop();
    }else{
        m_sprite->setProperty("frameRate", 4);
        m_decisionTimer->start();
    }
}

void WildPokemon::direction(int direction){
    m_currentDirection = direction%4;
    m_sprite->setProperty("direction",m_currentDirection);
};

void WildPokemon::startBattle(){
    m_moveTimer->disconnect();
    m_decisionTimer->disconnect();

    const int BOUNDARY_MARGIN = 16;
    const QRect& screen = Globals::screenGeometry();
    int screenRight = screen.x() + screen.width();
    int screenBottom = screen.y() + screen.height();

    QPoint delta(0, 0);

    if (y() < BOUNDARY_MARGIN*2.5) { //manual increase for top screen
        direction(2);
        delta += QPoint(0, BOUNDARY_MARGIN);
    }

    if (x() < BOUNDARY_MARGIN) {
        direction(3);
        delta += QPoint(BOUNDARY_MARGIN, 0);
    }

    if (y() + height() > screenBottom - BOUNDARY_MARGIN) {
        direction(0);
        delta += QPoint(0, -BOUNDARY_MARGIN);
    }

    if (x() + width() > screenRight - BOUNDARY_MARGIN) {
        direction(1);
        delta += QPoint(-BOUNDARY_MARGIN, 0);
    }

    if (!delta.isNull()) {
        setPosition(position() + delta);
    }

    Globals::getPlayer().iChooseYou(this);
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
    QPoint delta;
    switch (m_currentDirection) {
        case 0: delta = QPoint(0, -m_moveSpeed); break;
        case 1: delta = QPoint(-m_moveSpeed, 0); break;
        case 2: delta = QPoint(0, m_moveSpeed); break;
        case 3: delta = QPoint(m_moveSpeed, 0); break;
    }
    setPosition(position() + delta);
}

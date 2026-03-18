#include "Battle.h"
#include "DesktopScene.h"
#include "globals.h"
#include "pokemon_data.h"
#include <QQuickItem>
#include <qnamespace.h>
#include <QMouseEvent>
#include <QTimer>
Battle::Battle(WildPokemon* opp, const PokemonInfo* chosen_info, QWindow *parent)
    : DesktopScene(parent)
    , m_initialOppPos(opp->position())
    , m_opp_info(opp->info)
    , m_chosen_info(chosen_info)
{
    qDebug() << "Battle constructor called!";
    m_currentDirection = opp->m_currentDirection;
    // Load the PokemonSprite as root
    setSource(QUrl("qrc:/qml/BattleScene.qml"));
    m_battleScene = rootObject();
    assert(m_battleScene);

    m_battleScene->setProperty("direction",m_currentDirection);
    m_battleScene->setProperty("pokeMargin", m_pokeMargin);
    m_battleScene->setProperty("debugLines", Globals::DEBUG);
    m_opp = setupPokemon(m_opp_info, "opponent"); //these are the only valid strings
    m_chosen = setupPokemon(m_chosen_info, "player");//no enums here, only hopes and dreams

    initPosition();
    show();

    QTimer::singleShot(2, [this,opp]() {//just to make sure its all smooth
        opp->hide();
    });

    QObject* helper = new QObject(this);
    QObject::connect(m_battleScene, SIGNAL(runClicked()),
                    helper, SLOT(deleteLater()));
    QObject::connect(helper, &QObject::destroyed,
                    [this, opp]() { //this assumes opp is untouched
                        QPoint delta =  position() - m_origin;
                        opp->setPosition(opp->position() + delta);
                        opp->startRoaming();
                        opp->show();
                        this->close();
                    });

    qDebug() << "Window shown, visible:" << isVisible();
}

QQuickItem* Battle::setupPokemon(const PokemonInfo* info, const char* role) {
    QQuickItem* container = m_battleScene->property(role).value<QQuickItem*>();

    Q_ASSERT_X(container, "Battle::setupPokemon", "Container: '%1' is null".arg(role));

    // Set the basic sprite properties
    container->setProperty("spriteSheet", QString("qrc:/assets/HGSS/PokGen%1_transparent_reordered.png").arg(info->generation));
    container->setProperty("row", info->spriteId);
    container->setProperty("scaleFactor", Globals::SCALE);
    container->setProperty("debugLines", Globals::DEBUG);

    const SpriteInfo* spriteInfo = Globals::getSpriteInfo(info->spriteId, info->generation);

    // Calculate container size
    int width = Globals::SCALE * (spriteInfo->max_width + Globals::POKE_PADDING);
    int height = Globals::SCALE * (spriteInfo->max_height + Globals::POKE_PADDING);

    // Calculate positioning offsets
    int offsetX = Globals::SCALE * (32 - spriteInfo->max_width) / 2;
    int offsetY = Globals::SCALE * (32 - spriteInfo->max_height) / 2;

    // Set container properties
    container->setProperty("itemWidth", width);
    container->setProperty("itemHeight", height);
    container->setProperty("containerOffsetX", offsetX);
    container->setProperty("containerOffsetY", offsetY);

    // Position the sprite
    QMetaObject::invokeMethod(m_battleScene, "positionSpriteAndHealthbar", Q_ARG(QVariant, QVariant::fromValue(container)));

    return container;
}


void Battle::initPosition() {
    QQuickItem* rootItem = qobject_cast<QQuickItem*>(m_battleScene);
    QQuickItem* opponentItem = m_battleScene->findChild<QQuickItem*>("opponent");
        //has to be QPointF for the method, but since we set offets as integer in qml it gives smooth transition
    QPointF spriteOffset = opponentItem->mapToItem(rootItem, QPointF(0, 0));

    m_origin = m_initialOppPos + QPoint(-spriteOffset.x(), -spriteOffset.y());
    setPosition(m_origin);
}

void Battle::direction(int direction){m_currentDirection=direction;};
void Battle::handleDrag(bool isDragged){m_isDragged = isDragged;};

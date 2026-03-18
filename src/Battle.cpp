#include "Battle.h"
#include "DesktopScene.h"
#include "globals.h"
#include "pokemon_data.h"
#include <QQuickItem>
#include <qnamespace.h>
#include <QMouseEvent>
#include <cassert>

Battle::Battle(int opp_direction, QPoint opp_pos, const PokemonInfo* opp_info, const PokemonInfo* chosen_info, QWindow *parent)
    : DesktopScene(parent)
    , m_initialOppPos(opp_pos)
    , m_opp_info(opp_info)
    , m_chosen_info(chosen_info)
{
    qDebug() << "Battle constructor called!";
    m_currentDirection = opp_direction;
    // Load the PokemonSprite as root
    setSource(QUrl("qrc:/sprites/BattleScene.qml"));
    m_battleScene = rootObject();
    assert(m_battleScene);

    m_battleScene->setProperty("direction",m_currentDirection);
    m_battleScene->setProperty("pokeMargin", m_pokeMargin);
    m_battleScene->setProperty("debugLines", Globals::DEBUG);
    setupPokemon(m_opp_info, "opponent"); //these are the only valid strings
    setupPokemon(m_chosen_info, "player");//no enums here, only hopes and dreams



    initPosition();
    show();
    qDebug() << "Window shown, visible:" << isVisible();
}

void Battle::onBattleSceneLoaded(QVariant battleSceneItem) {
    /* QQuickItem* scene = battleSceneItem.value<QQuickItem*>(); */
    /* if (scene) { */
    /*     resize(scene->width(), scene->height()); */
    /*     qDebug() << "Battle scene loaded, resized to:" << scene->width() << "x" << scene->height(); */
    /* } */
}


void Battle::setupPokemon(const PokemonInfo* info, const char* role) {
    QObject* container = m_battleScene->property(role).value<QObject*>();

    Q_ASSERT_X(container, "Battle::setupPokemon", "Container: '%1' is null".arg(role));

    container->setProperty("spriteSheet", QString("qrc:/assets/HGSS/PokGen%1_transparent_reordered.png").arg(info->generation));
    container->setProperty("row", info->spriteId);
    container->setProperty("scaleFactor", Globals::SCALE);
    container->setProperty("debugLines", Globals::DEBUG);

    const SpriteInfo* spriteInfo = Globals::getSpriteInfo(info->spriteId, info->generation);
    int width = Globals::SCALE* (spriteInfo->max_width + Globals::POKE_PADDING);
    int height = Globals::SCALE * (spriteInfo->max_height + Globals::POKE_PADDING);
    int offsetX = Globals::SCALE*(32 - spriteInfo->max_width)/2;
    int offsetY = Globals::SCALE*(32 - spriteInfo->max_height)/2;

    container->setProperty("itemWidth", width);
    container->setProperty("itemHeight", height);
    container->setProperty("spriteOffsetX" , Globals::POKE_PADDING/2);
    container->setProperty("spriteOffsetY", Globals::POKE_PADDING/2);
    container->setProperty("containerOffsetX", offsetX);
    container->setProperty("containerOffsetY", offsetY);

    QMetaObject::invokeMethod(m_battleScene, "positionSprite", Q_ARG(QVariant, QVariant::fromValue(container)));
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

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

    m_battleScene->setProperty("spriteSheet", QString("qrc:/assets/HGSS/PokGen%1_transparent_reordered.png").arg(opp_info->generation));
    m_battleScene->setProperty("scaleFactor", Globals::SCALE);
    m_battleScene->setProperty("direction",m_currentDirection);

    setupPokemon(m_opp_info, m_chosen_info);


    initPosition();
    show();
    qDebug() << "Window shown, visible:" << isVisible();
}

void Battle::onBattleSceneLoaded(QVariant battleSceneItem) {
    QQuickItem* scene = battleSceneItem.value<QQuickItem*>();
    if (scene) {
        resize(scene->width(), scene->height());
        qDebug() << "Battle scene loaded, resized to:" << scene->width() << "x" << scene->height();
    }
}

void Battle::setupPokemon(const PokemonInfo* opp_info, const PokemonInfo* chosen_info) {
    if (!m_battleScene) return;

    // Set top-level properties directly
    m_battleScene->setProperty("opponentSpriteSheet",
        QString("qrc:/assets/HGSS/PokGen%1_transparent_reordered.png").arg(opp_info->generation));
    m_battleScene->setProperty("opponentRow", opp_info->spriteId);

    m_battleScene->setProperty("playerSpriteSheet",
        QString("qrc:/assets/HGSS/PokGen%1_transparent_reordered.png").arg(chosen_info->generation));
    m_battleScene->setProperty("playerRow", chosen_info->spriteId);

    m_battleScene->setProperty("pokemonScale", Globals::SCALE);
    m_battleScene->setProperty("debugLines", Globals::DEBUG);
}

void Battle::handleDrag(bool isDragged){};
void Battle::initPosition(){
    int distance = m_currentDirection%2==0 ? 3*33 : 4*33;
    switch(m_currentDirection) {
        case 0:
            m_origin = m_initialOppPos;// + QPoint(0, -m_ui->height() + 32);
            /* m_chosen->setPosition(m_initialOppPos + QPoint(0, -distance)); */
            break;
        case 1:
            m_origin = m_initialOppPos ;//+ QPoint(-m_ui->width()/2, -m_ui->height()/2 + 32);
            /* m_chosen->setPosition(m_initialOppPos + QPoint(-distance, 0)); */
            break;
        case 2:
            m_origin = m_initialOppPos ;//+ QPoint(0, -32);
            /* m_chosen->setPosition(m_initialOppPos + QPoint(0, distance)); */
            break;
        case 3:
            m_origin = m_initialOppPos ;//+ QPoint(-32/2, -32);
            /* m_chosen->setPosition(m_opp->position() + QPoint(distance, 0)); */
            break;
    }

    QMetaObject::invokeMethod(m_ui, "set_chosen_side", Q_ARG(QVariant, m_currentDirection));
    QMetaObject::invokeMethod(m_ui, "swap_visibility");
    QMetaObject::invokeMethod(m_ui, "swap_visibility");

    setPosition(m_origin);
}
void Battle::direction(int direction){};

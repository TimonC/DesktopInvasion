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
    m_battleScene->setProperty("pokeMargin", m_pokeMargin);
    m_battleScene->setProperty("debugLines", Globals::DEBUG);
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
    Q_ASSERT_X(m_battleScene, "Battle::setupPokemon", "Battle scene is null");

    QObject* opponentSprite = m_battleScene->property("opponentSprite").value<QObject*>();
    QObject* playerSprite = m_battleScene->property("playerSprite").value<QObject*>();

    Q_ASSERT_X(opponentSprite, "Battle::setupPokemon", "Opponent sprite is null");
    Q_ASSERT_X(playerSprite, "Battle::setupPokemon", "Player sprite is null");

    opponentSprite->setProperty("spriteSheet", QString("qrc:/assets/HGSS/PokGen%1_transparent_reordered.png").arg(opp_info->generation));
    opponentSprite->setProperty("row", opp_info->spriteId);
    opponentSprite->setProperty("scaleFactor", Globals::SCALE);
    opponentSprite->setProperty("debugLines", Globals::DEBUG);

    playerSprite->setProperty("spriteSheet", QString("qrc:/assets/HGSS/PokGen%1_transparent_reordered.png").arg(chosen_info->generation));
    playerSprite->setProperty("row", chosen_info->spriteId);
    playerSprite->setProperty("scaleFactor", Globals::SCALE);
    playerSprite->setProperty("debugLines", Globals::DEBUG);
}
void Battle::handleDrag(bool isDragged){m_isDragged = isDragged;};

void Battle::initPosition(){
    int textBoxHeight = 32 * 1.6 * Globals::SCALE;
    int padding = 2 * Globals::SCALE;

    switch(m_currentDirection) {
        case 0: //opp look up
            m_origin = m_initialOppPos + QPoint(-m_pokeMargin, -height() + textBoxHeight - m_pokeMargin - padding);
            break;
        case 1: //opp look left
            m_origin = m_initialOppPos + QPoint(m_pokeMargin - width(),height()/2 - textBoxHeight + padding);
            break;
        case 2: //opp look down
            m_origin = m_initialOppPos + QPoint(-m_pokeMargin, -m_pokeMargin - padding);
            break;
        case 3: //opp look right
            m_origin = m_initialOppPos + QPoint(-m_pokeMargin,height()/2 -textBoxHeight + padding);
            break;
    }
    setPosition(m_origin);
}
void Battle::direction(int direction){};

#include "Battle.h"
#include "DesktopScene.h"
#include "globals.h"
#include "pokemon_data.h"
#include <QQuickItem>
#include <qnamespace.h>
#include <QMouseEvent>

Battle::Battle(int opp_direction, QPoint opp_pos, const PokemonInfo* opp_info, const PokemonInfo* chosen_info, QWindow *parent)
    : DesktopScene(parent)
    , m_initialOppPos(opp_pos)
    , m_opp_info(opp_info)
    , m_chosen_info(chosen_info)
{
    qDebug() << "Battle constructor called!";
    m_currentDirection = opp_direction;

    // Load the PokemonSprite as root
    setSource(QUrl("qrc:/sprites/PokemonSprite.qml"));


    m_opp = rootObject();
    if (m_opp) {
        qDebug() << "Root object loaded successfully";

        m_opp->setProperty("spriteSheet", QString("qrc:/assets/HGSS/PokGen%1_transparent_reordered.png").arg(opp_info->generation));
        m_opp->setProperty("scaleFactor", Globals::SCALE);
        m_opp->setProperty("row", opp_info->spriteId);
        m_opp->setProperty("debugLines", Globals::DEBUG);
        m_opp->setProperty("direction",m_currentDirection);

        // Activate the battle scene

    QObject::connect(m_opp, SIGNAL(battleSceneLoaded(QVariant)),
                     this, SLOT(onBattleSceneLoaded(QVariant)));

    QMetaObject::invokeMethod(m_opp, "loadBattleScene", Q_ARG(QVariant, opp_direction));
        /* QMetaObject::invokeMethod(m_opp, "loadBattleScene", Q_ARG(QVariant, m_currentDirection)); */
        /* m_opp->setProperty("chosenSide", opp_direction); */

        qDebug() << "Properties set, battleActive:" << m_opp->property("battleActive");
    } else {
        qDebug() << "Failed to load root object!";
    }

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

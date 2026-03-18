#include "Battle.h"
#include "DesktopScene.h"
#include "globals.h"
#include "pokemon_data.h"
#include <QQuickItem>
#include <qnamespace.h>
#include <QMouseEvent>

Battle::Battle(int opp_direction, QPoint opp_pos, const PokemonInfo* opp, const PokemonInfo* chosen, QWindow *parent)
    : DesktopScene(parent)
    , m_initialOppPos(opp_pos)
    , m_opp(opp)
    , m_chosen(chosen)
{
    qDebug() << "Battle constructor called!";
    qDebug() << m_opp;
    m_currentDirection = opp_direction;

    setSource(QUrl("qrc:/sprites/BattleScene.qml"));

    QQuickItem *ui = rootObject();
    m_ui = ui;

    m_ui->setProperty("debugLines", Globals::DEBUG);

    /* QQuickItem* runButton = ui->property("runButton").value<QQuickItem*>(); */
    /* connect(runbutton, signal(clicked()), this, slot(run())); */

    /* QQuickItem* attackButton = ui->property("attackButton").value<QQuickItem*>(); */
    /* connect(attackButton, SIGNAL(clicked()), this, SLOT(attack())); */

    initPosition();
    show();
}
void Battle::handleDrag(bool isDragged){};
void Battle::initPosition(){
    int distance = m_currentDirection%2==0 ? 3*33 : 4*33;
    switch(m_currentDirection) {
        case 0:
            m_origin = m_initialOppPos + QPoint(25, -m_ui->height()/2 + 30);
            /* m_chosen->setPosition(m_initialOppPos + QPoint(0, -distance)); */
            break;
        case 1:
            m_origin = m_initialOppPos + QPoint(-m_ui->width()/2+8, 16);
            /* m_chosen->setPosition(m_initialOppPos + QPoint(-distance, 0)); */
            break;
        case 2:
            m_origin = m_initialOppPos + QPoint(+25, 50);
            /* m_chosen->setPosition(m_initialOppPos + QPoint(0, distance)); */
            break;
        case 3:
            m_origin = m_initialOppPos + QPoint(28, 16);
            /* m_chosen->setPosition(m_opp->position() + QPoint(distance, 0)); */
            break;
    }

    QMetaObject::invokeMethod(m_ui, "set_chosen_side", Q_ARG(QVariant, m_currentDirection));
    QMetaObject::invokeMethod(m_ui, "swap_visibility");
    QMetaObject::invokeMethod(m_ui, "swap_visibility");

    setPosition(m_origin);
}
void Battle::direction(int direction){};

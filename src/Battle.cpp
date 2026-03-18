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
/* void Battlescene::run(){ */
/*     setVisible(false); */
/*     m_corners->hide(); */
/*     m_chosen->setVisible(false); */
/*     m_chosen->m_inABattle = false; */
/*     Globals::getPlayer().m_pokemonAvailable = true; */
/*     m_opp->startRoaming(); */
/* } */

/* void Battlescene::attack(){ */
/*     m_chosen->useMove(); */
/*     m_opp->attacked(); */
/* } */

/* void Battlescene::updateTextbar(const std::string &text){ */
/*     if (m_ui) { */
/*         QString qText = QString::fromStdString(text); */
/*         QMetaObject::invokeMethod(m_ui, "update_text_bar", Q_ARG(QVariant, qText)); */
/*     } */
/* } */

/* QQuickView* Battlescene::initCorners(){ */
/*     int lft = std::min(m_opp->position().x(), m_chosen->position().x()); */
/*     int top = std::min(m_opp->position().y(), m_chosen->position().y()); */
/*     int boxHeight; int boxWidth; */
/*     if(m_direction % 2 == 0){ */
/*         boxWidth  = x() + m_ui->width()  - lft; */
/*         boxHeight = y() + m_ui->height() -  std::min(m_opp->position().y() , m_chosen->position().y()); */
/*     }else{ */
/*         boxWidth  = std::max(m_opp->position().x() + m_opp->width(), m_chosen->position().x() + m_chosen->width()) - lft; */
/*         boxHeight = y() + height() - top; */
/*     } */
/*     QQuickView *corners = new QQuickView(nullptr); */
/*     corners->setFlags(Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus | Qt::FramelessWindowHint | Qt::WindowTransparentForInput); */
/*     corners->setColor(Qt::transparent); */
/*     corners->setPosition(QPoint(lft,top)); */
/*     corners->setSource(QUrl("qrc:/sprites/Corners.qml")); */
/*     corners->setWidth(boxWidth); */
/*     corners->setHeight(boxHeight); */

/*     m_cornerSize = QPoint(boxWidth, boxHeight); */
/*     corners->rootObject()->setProperty("debugLines", Globals::DEBUG); */
/*     corners->show(); */

/*     return corners; */
/* } */


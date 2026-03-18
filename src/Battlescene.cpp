#include "Battlescene.h"
#include "globals.h"
#include <QQuickItem>
#include <qnamespace.h>

Battlescene::Battlescene(Pokemon *opp, Pokemon *chosen, QWindow *parent)
    : QQuickView(parent)
    , m_opp(opp)
    , m_chosen(chosen)
{
    setFlags(     Qt::WindowStaysOnTopHint
                | Qt::Tool
                | Qt::WindowDoesNotAcceptFocus
                | Qt::FramelessWindowHint);
    setColor(Qt::transparent);

    m_opp->setFlag(Qt::WindowTransparentForInput);
    m_chosen->setFlag(Qt::WindowTransparentForInput);

    setSource(QUrl("qrc:/sprites/BattleScene.qml"));

    QQuickItem *ui = rootObject();
    m_ui = ui;

    QQuickItem* runButton = ui->property("runButton").value<QQuickItem*>();
    connect(runButton,  SIGNAL(clicked()), this, SLOT(run()));

    QQuickItem* attackButton = ui->property("attackButton").value<QQuickItem*>();
    connect(attackButton,  SIGNAL(clicked()), this, SLOT(attack()));


    QQuickItem* mouseArea = ui->property("mouseArea").value<QQuickItem*>();

    int distance = m_opp->direction()%2==0 ? 3*33 : 4*33;
    switch(m_opp->direction()) {
        case 0:
            m_origin = m_opp->position() + QPoint(0, -ui->height()/2-14);
            m_chosen->setPosition(m_opp->position() + QPoint(0, -distance));
            m_chosen->direction(2);
            break;
        case 1:
            m_origin = m_opp->position() + QPoint(-ui->width()/2-16, -16);
            m_chosen->setPosition(m_opp->position() + QPoint(-distance, 0));
            m_chosen->direction(3);
            break;
        case 2:
            m_origin = m_opp->position() + QPoint(0, 0);
            m_chosen->setPosition(m_opp->position() + QPoint(0, distance));
            m_chosen->direction(0);
            break;
        case 3:
            m_origin = m_opp->position() + QPoint(0, -30);
            m_chosen->setPosition(m_opp->position() + QPoint(distance, 0));
            m_chosen->direction(1);
            break;
    }
    /* m_opp->setPosition(m_opp->position()-m_origin); */

    // Set m_chosen side
    QMetaObject::invokeMethod(ui, "set_chosen_side", Q_ARG(QVariant, m_opp->direction()));
    QMetaObject::invokeMethod(ui, "swap_visibility");
    QMetaObject::invokeMethod(ui, "swap_visibility");
    m_direction = m_chosen->direction();
    setPosition(m_origin);

    show();
}

void Battlescene::run(){
    setVisible(false);
    m_chosen->setVisible(false);

    getPlayer().m_pokemonAvailable = true;
    m_opp->startRoaming();
    m_chosen->m_inABattle = false;
}

void Battlescene::attack(){
    m_chosen->useMove();
    m_opp->attacked();
}

void Battlescene::updateTextbar(const std::string &text){
    if (m_ui) {
        QString qText = QString::fromStdString(text);
        QMetaObject::invokeMethod(m_ui, "update_text_bar", Q_ARG(QVariant, qText));
    }
};

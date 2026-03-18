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
    switch(m_opp->direction()) { //Very bad no good ugly manually-tuned positioning for the Battlescene
        case 0:
            m_origin = m_opp->position() + QPoint(25, -ui->height()/2 + 30);
            m_chosen->setPosition(m_opp->position() + QPoint(0, -distance));
            m_chosen->direction(2);
            break;
        case 1:
            m_origin = m_opp->position() + QPoint(-ui->width()/2+8, 16);
            m_chosen->setPosition(m_opp->position() + QPoint(-distance, 0));
            m_chosen->direction(3);
            break;
        case 2:
            m_origin = m_opp->position() + QPoint(+25, 50);
            m_chosen->setPosition(m_opp->position() + QPoint(0, distance));
            m_chosen->direction(0);
            break;
        case 3:
            m_origin = m_opp->position() + QPoint(28, 16);
            m_chosen->setPosition(m_opp->position() + QPoint(distance, 0));
            m_chosen->direction(1);
            break;
    }

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


void Battlescene::mousePressEvent(QMouseEvent* event) {
    if (Qt::LeftButton) {
        m_oldpos = event->globalPosition().toPoint();
        m_dragging = true;

        // Check if click is on any button
        QPoint localPos = mapFromGlobal(m_oldpos);
        QQuickItem* runButton = m_ui->property("runButton").value<QQuickItem*>();
        QQuickItem* attackButton = m_ui->property("attackButton").value<QQuickItem*>();
        QQuickItem* switchButton = m_ui->property("switchButton").value<QQuickItem*>();
        QQuickItem* catchButton = m_ui->property("catchButton").value<QQuickItem*>();

        if ((runButton && runButton->contains(runButton->mapFromScene(localPos))) ||
            (attackButton && attackButton->contains(attackButton->mapFromScene(localPos))) ||
            (switchButton && switchButton->contains(switchButton->mapFromScene(localPos))) ||
            (catchButton && catchButton->contains(catchButton->mapFromScene(localPos)))) {
            m_dragging = false;
        }
    }
}

void Battlescene::mouseMoveEvent(QMouseEvent* event){
    if (m_dragging && (event->buttons() & Qt::LeftButton)){
        QPoint currentPos = event->globalPosition().toPoint();
        drag(currentPos-m_oldpos);
        m_oldpos = currentPos;
    }
}
void Battlescene::drag(QPoint delta){
    QPoint pos = position();
    if (pos.x() + delta.x() < 0 || pos.x() + delta.x() > getScreenGeometry().width() - width())
        delta.setX(0);
    if (pos.y() + delta.y() < 0 || pos.y() + delta.y() > getScreenGeometry().height() - height())
        delta.setY(0);

    setPosition(pos + delta);
    m_chosen->movePos(delta, false);
    m_opp->movePos(delta, false);
}

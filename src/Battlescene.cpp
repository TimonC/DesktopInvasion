#include "Battlescene.h"
#include <QQuickItem>
#include <qnamespace.h>

Battlescene::Battlescene(Pokemon *opp, Pokemon *chosen, QWindow *parent)
    : QQuickView(parent)
{
    setFlags(     Qt::WindowStaysOnTopHint
                | Qt::Tool
                | Qt::WindowDoesNotAcceptFocus
                | Qt::FramelessWindowHint);
    setColor(Qt::transparent);

    opp->setFlag(Qt::WindowTransparentForInput);
    chosen->setFlag(Qt::WindowTransparentForInput);

    setSource(QUrl("qrc:/sprites/BattleScene.qml"));

    QQuickItem *ui = rootObject();
    m_ui = ui;

    // Position calculation
    int distance = opp->direction()%2==0 ? 3*32 : 4*32;
    /* int distance = 3*32; */
    switch(opp->direction()) {
        case 0:
            m_origin = opp->position() + QPoint(0, -ui->height()/2-14);
            chosen->setPosition(opp->position() + QPoint(0, -distance));
            chosen->direction(2);
            break;
        case 1:
            m_origin = opp->position() + QPoint(-ui->width()/2-16, -16);
            chosen->setPosition(opp->position() + QPoint(-distance, 0));
            chosen->direction(3);
            break;
        case 2:
            m_origin = opp->position() + QPoint(0, 0);
            chosen->setPosition(opp->position() + QPoint(0, distance));
            chosen->direction(0);
            break;
        case 3:
            m_origin = opp->position() + QPoint(0, -30);
            chosen->setPosition(opp->position() + QPoint(distance, 0));
            chosen->direction(1);
            break;
    }

    // Set chosen side
    QMetaObject::invokeMethod(ui, "set_chosen_side", Q_ARG(QVariant, opp->direction()));
    QMetaObject::invokeMethod(ui, "swap_visibility");
    QMetaObject::invokeMethod(ui, "swap_visibility");
    m_direction = chosen->direction();
    setPosition(m_origin);

    show();
}

void Battlescene::updateTextbar(const std::string &text){
    if (m_ui) {
        QString qText = QString::fromStdString(text);
        QMetaObject::invokeMethod(m_ui, "update_text_bar", Q_ARG(QVariant, qText));
    }
};

#include "Battlescene.h"
#include <QQuickItem>

Battlescene::Battlescene(Pokemon *opp, Pokemon *chosen, QWindow *parent)
    : QQuickView(parent)
{
    setFlags(Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
    setColor(Qt::transparent);

    setSource(QUrl("qrc:/sprites/BattleScene.qml"));

    QQuickItem *ui = rootObject();
    m_ui = ui;
    // Position calculation
    int distance = 100;
    switch(opp->direction()) {
        case 0:
            m_origin = opp->position(); //+ QPoint(-32, 32*2.5);
            chosen->setPosition(opp->position() + QPoint(0, -distance));
            chosen->direction(2);
            break;
        case 1:
            m_origin = opp->position(); // QPoint(-distance + 32/2, 32*2.5);
            chosen->setPosition(opp->position() + QPoint(-distance, 0));
            chosen->direction(3);
            break;
        case 2:
            m_origin = opp->position();// + QPoint(-32, distance + 32*2.5);
            chosen->setPosition(opp->position() + QPoint(0, distance));
            chosen->direction(0);
            break;
        case 3:
            m_origin = opp->position();// + QPoint(32/2, 32*2.5);
            chosen->setPosition(opp->position() + QPoint(distance, 0));
            chosen->direction(1);
            break;
    }

    // Set chosen side
    QMetaObject::invokeMethod(ui, "set_chosen_side", Q_ARG(QVariant, opp->direction()));
    QMetaObject::invokeMethod(ui, "set_buttons_visible", Q_ARG(QVariant, true));

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

#include <Battlescene.h>
#include <qquickitem.h>
#include <qquickview.h>

Battlescene::Battlescene(Pokemon *opp, Pokemon *chosen, QWindow *parent)
    : QQuickView(parent){

    setFlags( Qt::WindowStaysOnTopHint
            | Qt::Tool
            | Qt::WindowDoesNotAcceptFocus
            | Qt::FramelessWindowHint);
    setColor(Qt::transparent);

    int distance = 100;
    setSource(QUrl("qrc:/sprites/BattleText.qml"));
    QQuickItem *textBar = rootObject();

    switch(opp->direction()){
        case 0:
            m_origin = opp->position() + QPoint(-32,32*2.5);
            chosen->setPosition(opp->position() + QPoint(0, -distance));
            chosen->direction(2);
            break;
        case 1:
            m_origin = opp->position() + QPoint(-distance + 32/2,32*2.5);
            chosen->setPosition(opp->position() + QPoint(-distance, 0));
            chosen->direction(3);
            break;
        case 2:
            m_origin = opp->position() + QPoint(-32,distance + 32*2.5);
            chosen->setPosition(opp->position() + QPoint(0, distance));
            chosen->direction(0);
            break;
        case 3:
            m_origin = opp->position() + QPoint(32/2, 32*2.5);
            chosen->setPosition(opp->position() + QPoint(distance, 0));
            chosen->direction(1);
            break;
    }

    connect(this, &Battlescene::updateTextbar, textBar, [textBar](const QString &text) {
        textBar->setProperty("text", text);
    });

    m_direction = chosen->direction();
    setPosition(m_origin);

    show();
};
QString updateTextBar(const QString &text){
    return text;
};

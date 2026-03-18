#include <Battlescene.h>

Battlescene::Battlescene(Pokemon *opp, Pokemon *chosen, QWindow *parent)
    : QQuickView(parent){

    setFlags( Qt::WindowStaysOnTopHint
            | Qt::Tool
            | Qt::WindowDoesNotAcceptFocus
            | Qt::FramelessWindowHint);
    setColor(Qt::transparent);

    int distance = 100;

    switch(opp->direction()){
        case 0:
            m_origin = opp->position() + QPoint(0,0);
            chosen->setPosition(opp->position() + QPoint(0, -distance));
            chosen->direction(2);
            break;
        case 1:
            m_origin = opp->position();
            chosen->setPosition(opp->position() + QPoint(-distance, 0));
            chosen->direction(3);
            break;
        case 2:
            m_origin = opp->position();
            chosen->setPosition(opp->position() + QPoint(0, distance));
            chosen->direction(0);
            break;
        case 3:
            m_origin = opp->position();
            chosen->setPosition(opp->position() + QPoint(distance, 0));
            chosen->direction(1);
            break;
    }
    m_direction = chosen->direction();


   setWidth(32*4);
   setHeight(32*4);

   setPosition(m_origin);
};

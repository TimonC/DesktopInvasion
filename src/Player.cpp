#include "Player.h"
#include "Pokemon.h"

Pokemon* Player::iChooseYou(QPoint opp_spot, int opp_direction){
    m_chosen = std::make_unique<Pokemon>(nullptr, random()%100);
    int distance = 100;

    switch(opp_direction){
        case 0:
            m_chosen->setPosition(opp_spot + QPoint(0, -distance));
            m_chosen->setDirection(2);
            break;
        case 1:
            m_chosen->setPosition(opp_spot + QPoint(-distance, 0));
            m_chosen->setDirection(3);
            break;
        case 2:
            m_chosen->setPosition(opp_spot + QPoint(0, distance));
            m_chosen->setDirection(0);
            break;
        case 3:
            m_chosen->setPosition(opp_spot + QPoint(distance, 0));
            m_chosen->setDirection(1);
            break;
    }

    m_chosen->show();

    qDebug() << "I choose you!";
    return m_chosen.get();
};


#include "Player.h"
#include "Pokemon.h"

Player::Player(QObject* parent) : QObject(parent){
    qDebug() << "Player constructor called!";
    m_party[0] = std::make_unique<Pokemon>(nullptr, random()%100);
};

Pokemon* Player::iChooseYou(QPoint opp_spot, int opp_direction){
    Pokemon* chosen = m_party[0]->get();
    int distance = 100;

    switch(opp_direction){
        case 0:
            chosen->setPosition(opp_spot + QPoint(0, -distance));
            chosen->setDirection(2);
            break;
        case 1:
            chosen->setPosition(opp_spot + QPoint(-distance, 0));
            chosen->setDirection(3);
            break;
        case 2:
            chosen->setPosition(opp_spot + QPoint(0, distance));
            chosen->setDirection(0);
            break;
        case 3:
            chosen->setPosition(opp_spot + QPoint(distance, 0));
            chosen->setDirection(1);
            break;
    }

    chosen->show();

    qDebug() << "I choose you!";
    return chosen;
};


#include "Player.h"
#include <memory>

Player::Player(QObject* parent) : QObject(parent){
    qDebug() << "Player constructor called!";
    m_party[0] = std::make_unique<Pokemon>(nullptr, random()%100);
};

Pokemon* Player::iChooseYou(Pokemon *opp){
    m_inABattle = true;

    Pokemon* chosen = m_party[0].value().get();

      m_battlescene  =  std::make_unique<Battlescene>(opp, chosen);

    chosen->show();
    m_battlescene.value()->show();
    qDebug() << "I choose you!";
    return chosen;
};


#include "Player.h"
#include "Pokemon.h"

Pokemon* Player::iChooseYou(){
    Pokemon* chosen = new Pokemon(nullptr, 0);
    chosen->setDirection(0);
    chosen->show();
    qDebug() << "I choose you!";
    return chosen;
};

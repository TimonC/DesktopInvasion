#include "Player.h"
#include "Pokemon.h"

Pokemon* Player::iChooseYou(int partyId, QPoint mySpot, int direction){
    Pokemon* chosen = new Pokemon(nullptr, partyId);
    chosen->setDirection(direction);

    return chosen;
};

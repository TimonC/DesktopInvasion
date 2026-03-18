#ifndef PLAYER_H
#define PLAYER_H

#include "Pokemon.h"

class Player{
    Q_OBJECT

private:
        Player() {};

public:
        // C++ 11
        // =======
        // We can use the better technique of deleting the methods
        // we don't want. https://stackoverflow.com/questions/1008019/how-do-you-implement-the-singleton-design-pattern
        Player(Player const&)          = delete;
        void operator=(Player const&)  = delete;

        static Player& getInstance(){
            static Player instance;
            return instance;
        }

public slots:
        Pokemon* iChooseYou(int partyId, QPoint mySpot, int direction);
};

#endif

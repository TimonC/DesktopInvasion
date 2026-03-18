#ifndef PLAYER_H
#define PLAYER_H

#include "Pokemon.h"
#include <QObject>

using namespace std;
class Player: public QObject{
    Q_OBJECT

public:
    explicit Player(QObject* parent = nullptr);
    Pokemon* iChooseYou(QPoint opp_spot, int opp_direction);
    bool inABattle(){return _inABattle;};

private:
    bool _inABattle = false;
    array<optional<unique_ptr<Pokemon>>, 6> m_party;
};

#endif

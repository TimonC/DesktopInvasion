#ifndef PLAYER_H
#define PLAYER_H

#include "Pokemon.h"
#include <QObject>

using namespace std;
class Player: public QObject{
    Q_OBJECT

public:
    explicit Player(QObject* parent = nullptr);
    Pokemon* iChooseYou(QPoint opp_spot, int direction);

private:
    array<optional<unique_ptr<Pokemon>>, 6> m_party;
};

#endif

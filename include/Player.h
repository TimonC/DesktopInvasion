#ifndef PLAYER_H
#define PLAYER_H

#include "Pokemon.h"
#include <QObject>

class Player: public QObject{
    Q_OBJECT

public:
    explicit Player(QObject* parent = nullptr) : QObject(parent) {qDebug() << "Player constructor called!";};
    Pokemon* iChooseYou(QPoint opp_spot, int direction);

private:
    Pokemon* m_chosen;
};

#endif

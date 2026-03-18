#ifndef PLAYER_H
#define PLAYER_H

#include <Battlescene.h>
#include <Pokemon.h>
#include <QObject>

using namespace std;
class Player: public QObject{
    Q_OBJECT

public:
    explicit Player(QObject* parent = nullptr);
    Pokemon* iChooseYou(Pokemon* opp);
    bool inABattle(){return m_inABattle;};

private:
    bool m_inABattle = false;
    array<optional<unique_ptr<Pokemon>>, 6> m_party;
    optional<unique_ptr<Battlescene>> m_battlescene;
};

#endif

#ifndef PLAYER_H
#define PLAYER_H

/* #include <Battlescene.h> */
#include <QObject>

using namespace std;
class Player: public QObject{
    Q_OBJECT

public:
    explicit Player(QObject* parent = nullptr);
    /* Pokemon* iChooseYou(Pokemon* opp); */
    bool m_pokemonAvailable = false;

private:
    /* void _iChooseYou(Pokemon* opp, Pokemon* chosen); */
    /* array<optional<unique_ptr<Pokemon>>, 6> m_party; */
    /* std::vector<std::unique_ptr<Battlescene>> m_activeBattles; */
};

#endif

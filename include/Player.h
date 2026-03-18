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
    bool pokemonAvailable(){return m_pokemonAvailable;};

private:
    void _iChooseYou(Pokemon* opp, Pokemon* chosen);
    bool m_pokemonAvailable = false;
    array<optional<unique_ptr<Pokemon>>, 6> m_party;
    std::unordered_map<Pokemon*, std::unique_ptr<Battlescene>> m_activeBattles;
};

#endif

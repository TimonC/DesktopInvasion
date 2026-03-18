#include "Battlescene.h"
#include <Player.h>
#include <memory>

Player::Player(QObject* parent) : QObject(parent){
    qDebug() << "Player constructor called!";
    m_party[0] = std::make_unique<Pokemon>(nullptr, std::rand()%120);
    m_party[1] = std::make_unique<Pokemon>(nullptr, std::rand()%120);
    /* m_party[2] = std::make_unique<Pokemon>(nullptr, random()%100); */
    m_pokemonAvailable = true;
};
void Player::_iChooseYou(Pokemon* opp, Pokemon* chosen){
    chosen->m_inABattle = true;
    auto battle = std::make_unique<Battlescene>(opp, chosen);
    battle->updateTextbar("It's a battle...!");

    opp->raise();
    chosen->show();
    battle->show();
    battle->raise();
    m_activeBattles.push_back(std::move(battle));

    qDebug() << "I choose you!";
}

Pokemon* Player::iChooseYou(Pokemon *opp){
    Pokemon* chosen = nullptr;
    m_pokemonAvailable = false;

    auto it = m_party.begin();
    while(it!=m_party.end()){
        if(!it->has_value() || it->value()->m_inABattle){
            it++;
            continue;
        }

        if(!chosen){
            chosen = it->value().get();
            _iChooseYou(opp, chosen);
        }else{
            m_pokemonAvailable = true;
        }

        it++;
    }

    if(chosen) return chosen;
    assert(false && "No available Pokemon in party");
    return nullptr;
};


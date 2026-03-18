#include <QGuiApplication>
#include "WildPokemon.h"
#include "Player.h"
extern Player* player = new Player();
int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    WildPokemon pok1(nullptr, std::rand() % 100);
    WildPokemon pok2(nullptr, std::rand() % 100);
    WildPokemon pok3(nullptr, std::rand() % 120);

    /* QObject::connect(&pok1, &WildPokemon::startBattle, player, &Player::iChooseYou); */
    /* QObject::connect(&pok2, &WildPokemon::startBattle, player, &Player::iChooseYou); */
    /* QObject::connect(&pok3, &WildPokemon::startBattle, player, &Player::iChooseYou); */

    pok1.show();
    pok2.show();
    pok3.show();

    return app.exec();
}

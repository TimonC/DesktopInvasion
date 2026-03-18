#include <QGuiApplication>
#include "WildPokemon.h"
#include "Player.h"

extern Player* player = new Player();

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    WildPokemon pok1(nullptr, std::rand() % 100);
    pok1.show();

    return app.exec();
}

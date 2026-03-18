#include "globals.h"
#include "Player.h"
#include <QGuiApplication>
#include <qwindow.h>

Player& getPlayer() {
    static std::unique_ptr<Player> player = std::make_unique<Player>();
    return *player;
}

const PokemonInfo* findPokemonBySpriteId(int spriteId) {
    for (int i = 0; i < kPokemonCount; ++i) {
        if (kPokemonList[i].spriteId == spriteId) {
            return &kPokemonList[i]; // return pointer to the entry
        }
    }
    qFatal("Pokemon with spriteId %d not found!", spriteId);
    return nullptr; // this line won't be reached
}

QRect screenSize(){
    static QRect availableScreen = QGuiApplication::primaryScreen()->availableGeometry();
    return availableScreen;
}

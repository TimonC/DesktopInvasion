#include "globals.h"
#include "Player.h"
#include <QGuiApplication>
#include <qwindow.h>

Player& getPlayer() {
    static std::unique_ptr<Player> player = std::make_unique<Player>();
    return *player;
}


QRect screenSize(){
    static QRect availableScreen = QGuiApplication::primaryScreen()->availableGeometry();
    return availableScreen;
}

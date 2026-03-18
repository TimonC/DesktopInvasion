#include "globals.h"
#include "Player.h"
#include <QGuiApplication>

Player& getPlayer() {
    static std::unique_ptr<Player> player = std::make_unique<Player>();
    return *player;
}

QRect& getScreenGeometry() {
    static QRect geometry = QGuiApplication::primaryScreen()->geometry();
    return geometry;
}

int screenWidth(){
    static int width = QGuiApplication::primaryScreen()->geometry().width();
    return width;
}

int screenHeight(){
    static int height = QGuiApplication::primaryScreen()->geometry().height();
    return height;
}

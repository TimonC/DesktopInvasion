#include "globals.h"
#include "Player.h"
#include <QGuiApplication>
#include <qwindow.h>

Player& getPlayer() {
    static std::unique_ptr<Player> player = std::make_unique<Player>();
    return *player;
}


QPoint screenSize(){
    static QPoint screenSize = QPoint(QGuiApplication::primaryScreen()->geometry().width(), QGuiApplication::primaryScreen()->geometry().height());
    return screenSize;
}


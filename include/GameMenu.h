#ifndef GAMEMENU_H
#define GAMEMENU_H

#include <QQuickView>

class GameMenu: QQuickView{
    Q_OBJECT

public:
    explicit GameMenu(QWindow *parent = nullptr);

};

#endif

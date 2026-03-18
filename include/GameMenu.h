#ifndef GAMEMENU_H
#define GAMEMENU_H

#include <QQuickView>

class GameMenu: public QQuickView{
    Q_OBJECT

public:
    explicit GameMenu(QWindow *parent = nullptr);
    QQuickItem* m_menu;

};

#endif

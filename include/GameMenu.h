#ifndef GAMEMENU_H
#define GAMEMENU_H

#include <QQuickView>
#include <qtmetamacros.h>

class GameMenu: public QQuickView{
    Q_OBJECT

public:
    explicit GameMenu(QWindow *parent = nullptr);
    QQuickItem* m_menu;
    void activate();

signals:
    void menuClosed();

protected:
    bool event(QEvent *event) override;

};

#endif

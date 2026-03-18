#ifndef HITBOX_H
#define HITBOX_H

#include <QQuickView>
#include <QMouseEvent>

class Hitbox : public QQuickView
{
    Q_OBJECT

public:
    explicit Hitbox(QWindow *parent = nullptr);
    void showButton(bool show = true);
    QQuickItem* m_mouseArea;
    QQuickItem* m_battleButton;

protected:
    void mousePressEvent(QMouseEvent* event) override;


private:
};

#endif

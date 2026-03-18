#ifndef HITBOX_H
#define HITBOX_H

#include <QQuickView>
#include <QMouseEvent>
#include <qevent.h>

class Hitbox : public QQuickView
{
    Q_OBJECT

public:
    explicit Hitbox(QWindow *parent = nullptr);
    void showButton(bool show = true);
    QQuickItem* m_mouseArea;
    QQuickItem* m_battleButton;
    QPoint offset = QPoint(0,0);
signals:
    void drag(QPoint delta);
    void isDragged(bool dragged);
protected:
    QPoint m_oldpos = QPoint(0,0);
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent*  event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

};

#endif

#include "globals.h"
#include <DesktopScene.h>
#include <QWindow>
#include <QTimer>
#include <QQuickItem>

DesktopScene::DesktopScene(QWindow *parent)
    : QQuickView(parent)
    , m_currentDirection(0)
{
    setFlags(Qt::WindowStaysOnTopHint
        | Qt::Tool
        | Qt::WindowDoesNotAcceptFocus
        | Qt::FramelessWindowHint
        | Qt::BypassWindowManagerHint
        /* | Qt::WindowTransparentForInput */
        );
     setColor(Qt::transparent);
    //Short delay to ensure proper window size
    QTimer::singleShot(2, this, [this]() {
        m_width  = width();
        m_height = height();
    });
}


void DesktopScene::mouseMoveEvent(QMouseEvent* event){
    if(!m_isDragged) return;

    QPoint mousePos = event->globalPosition().toPoint();
    QPoint newPos = position()+ mousePos - m_oldMousePos;

    QRect screenGeom = Globals::screenGeometry();
    newPos.setX(qBound(screenGeom.left(), newPos.x(), screenGeom.right()- m_width));
    newPos.setY(qBound(screenGeom.top(), newPos.y(), screenGeom.bottom() - m_height));

    setPosition(newPos);
    m_oldMousePos = mousePos;
}


void DesktopScene::mousePressEvent(QMouseEvent* event){
    if(event->button()== Qt::LeftButton){
        handleDrag(true);
        m_oldMousePos = event->globalPosition().toPoint();
    }
}

void DesktopScene::mouseReleaseEvent(QMouseEvent* event){
    handleDrag(false);
}

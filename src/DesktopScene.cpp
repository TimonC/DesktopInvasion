#include "globals.h"
#include <DesktopScene.h>
#include <QWindow>
#include <QTimer>
#include <QQuickItem>
#include <macos_helper.h>


DesktopScene::DesktopScene(QWindow *parent)
    : QQuickView(parent)
    , m_currentDirection(0)
{
    setFlags( Qt::WindowStaysOnTopHint
            | Qt::Tool
            | Qt::WindowDoesNotAcceptFocus
            | Qt::FramelessWindowHint
            | Qt::BypassWindowManagerHint
    );
    setColor(Qt::transparent);

#ifdef Q_OS_MAC
    show();
    hide();
    setupMacOSWindow(reinterpret_cast<void*>(winId()));
#endif

    m_grabCursor = QCursor(QPixmap(":/assets/XY/grab.png"));
    m_pointerCursor = QCursor(QPixmap(":/assets/XY/pointer.png"), 6, 6);
    setCursor(m_pointerCursor);
}


void DesktopScene::mouseMoveEvent(QMouseEvent* event){
    if(m_isJumping) return;
    if(!m_isDragged) return;
    setCursor(m_grabCursor);

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
    if(cursor()==m_grabCursor) setCursor(m_pointerCursor);
    handleDrag(false);
}

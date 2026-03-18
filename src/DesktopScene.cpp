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

}


void DesktopScene::mouseMoveEvent(QMouseEvent* event){
    if(!m_isDragged) return;
    const QPoint newMousePos = event->globalPosition().toPoint();
    setPosition(position() + newMousePos - m_oldMousePos);
    m_oldMousePos = newMousePos;
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

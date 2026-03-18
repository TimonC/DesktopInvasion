#include "Hitbox.h"
#include <QQuickItem>
#include <globals.h>
Hitbox::Hitbox(QWindow *parent)
    : QQuickView(parent)
    , m_mouseArea(nullptr)
    , m_battleButton(nullptr)
{
    setSource(QUrl("qrc:/sprites/Hitbox.qml"));
    setFlags(Qt::WindowStaysOnTopHint
           | Qt::Tool
           | Qt::WindowDoesNotAcceptFocus
           | Qt::FramelessWindowHint);
    setColor(Qt::transparent);

    QQuickItem* hitbox_sprite = rootObject();
    hitbox_sprite->setProperty("debugLines", DEBUG);

    m_mouseArea = hitbox_sprite->property("mouseArea").value<QQuickItem*>();
    m_battleButton = hitbox_sprite->property("battleButton").value<QQuickItem*>();
}


void Hitbox::showButton(bool show){
    m_battleButton->setProperty("enabled", show);
    m_battleButton->setProperty("visible", show);
}

void Hitbox::mousePressEvent(QMouseEvent* event) {
    if (Qt::LeftButton) {
        m_oldpos = event->globalPosition().toPoint();
    }
    isDragged(true);
    // QQuickView::mousePressEvent(event);
}

void Hitbox::mouseMoveEvent(QMouseEvent* event){
    if (event->buttons() & Qt::LeftButton){
        QPoint currentPos = event->globalPosition().toPoint();
        emit drag(currentPos-m_oldpos);
        m_oldpos = currentPos;
    }

    // QQuickView::mouseMoveEvent(event);
}

void Hitbox::mouseReleaseEvent(QMouseEvent* event){
    isDragged(false);
}

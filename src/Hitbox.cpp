#include "Hitbox.h"
#include <QQuickItem>
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
    m_mouseArea = hitbox_sprite->property("mouseArea").value<QQuickItem*>();
    m_battleButton = hitbox_sprite->property("battleButton").value<QQuickItem*>();
}


void Hitbox::showButton(bool show){
    m_battleButton->setProperty("enabled", show);
    m_battleButton->setProperty("visible", show);
}


void Hitbox::mousePressEvent(QMouseEvent* event) {
    // Empty implementation for now
    QQuickView::mousePressEvent(event); // Call base class implementation
}

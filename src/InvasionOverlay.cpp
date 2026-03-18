#include "InvasionOverlay.h"
#include <QGuiApplication>
#include <QScreen>
#include <QQmlComponent>
#include <QQuickItem>
#include <qnamespace.h>
InvasionOverlay::InvasionOverlay(QWindow *parent)
    : QQuickView(parent)
{
    setFlags(Qt::FramelessWindowHint |
             Qt::WindowStaysOnTopHint |
             Qt::WindowTransparentForInput |
             Qt::Tool |
             Qt::WindowDoesNotAcceptFocus);

    setColor(Qt::transparent);  // makes QQuickView background transparent
    setResizeMode(SizeViewToRootObject);

    setSource(QUrl("InvasionCanvas.qml"));
    // Fullscreen on primary screen
    setGeometry(QGuiApplication::primaryScreen()->geometry());
}

void InvasionOverlay::addSprite(const QString& qmlFile, int x, int y){
    QQmlComponent component(engine(), QUrl(qmlFile));
    if (!component.isReady()) return;

    QQuickItem* sprite = qobject_cast<QQuickItem*>(component.create());
    if (!sprite) return;

    sprite->setParentItem(rootObject());  // Just use root directly
    sprite->setX(x);
    sprite->setY(y);
}


void InvasionOverlay::clearSprites(){
    QQuickItem* root = rootObject();
    if (!root) return;

    QList<QQuickItem*> children = root->childItems();
    for (QQuickItem* child : children) {
        child->deleteLater();
    }
}

#include "InvasionOverlay.h"
#include <QQmlComponent>
#include <QQuickItem>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

InvasionOverlay::InvasionOverlay(QWindow *parent)
    : QQuickView(parent)
{
    setFlags(Qt::FramelessWindowHint |
             Qt::WindowStaysOnTopHint |
             Qt::WindowTransparentForInput |
             Qt::Tool |
             Qt::WindowDoesNotAcceptFocus);

    setColor(Qt::transparent);
    setResizeMode(SizeViewToRootObject);
    setSource(QUrl("qrc:/InvasionCanvas.qml"));

    if (!rootObject()) {
        qDebug() << "Failed to load InvasionCanvas.qml!";
    }

    setGeometry(QGuiApplication::primaryScreen()->geometry());
}

void InvasionOverlay::addSprite(const QString &qmlFile, int x, int y){
    QQmlComponent component(engine(), QUrl(qmlFile));
    if (component.isError()) {
        qDebug() << "Error loading QML:" << component.errors();
        return;
    }

    QObject *obj = component.create();
    if (!obj) {
        qDebug() << "Failed to create QML object";
        return;
    }

    QQuickItem *item = qobject_cast<QQuickItem *>(obj);
    if (!item) {
        qDebug() << "Failed to cast to QQuickItem";
        return;
    }

    // Find the container
    QQuickItem *container = rootObject()->findChild<QQuickItem*>("spriteContainer");
    if (!container) {
        qDebug() << "spriteContainer not found!";
        return;
    }

    // Attach the sprite
    item->setParentItem(container);
    item->setX(x);
    item->setY(y);

    qDebug() << "Sprite added at" << x << y;
}

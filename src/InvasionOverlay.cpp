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

    setGeometry(QGuiApplication::primaryScreen()->geometry());
}



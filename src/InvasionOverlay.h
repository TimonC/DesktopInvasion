#ifndef INVASIONOVERLAY_H
#define INVASIONOVERLAY_H

#include <QQuickView>

class InvasionOverlay : public QQuickView
{
    Q_OBJECT

public:
    InvasionOverlay(QWindow *parent = nullptr);
    Q_INVOKABLE void addSprite(const QString& qmlFile, int x, int y);
    Q_INVOKABLE void clearSprites();
};

#endif

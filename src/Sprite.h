#ifndef SPRITE_H
#define SPRITE_H

#include <QObject>
#include <QGraphicsItem>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QDebug>
class Sprite : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)  // tell Qt MOC about the QGraphicsItem interface

public:
    explicit Sprite(QObject *parent = nullptr);

private slots:
    void nextFrame();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;

private:
    QTimer *timer;
    QPixmap *spriteImage;
    int currentFrame;
};

#endif


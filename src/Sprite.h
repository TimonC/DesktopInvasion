#ifndef SPRITE_H
#define SPRITE_H

#include <QObject>
#include <QGraphicsItem>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
/* #include <qgraphicsscene.h> */
/* #include <qobjectdefs.h> */

class Sprite: public QObject, public QGraphicsItem{
    Q_OBJECT

    public:
       explicit Sprite(QObject *parent = 0);

    private slots:
        void nextFrame();

    private:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        QRectF boundingRect() const;
        QTimer *timer;
        QPixmap *spriteImage;
        int currentFrame;
};

#endif

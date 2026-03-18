#include "Sprite.h"
#include <qglobal.h>
#include <qgraphicsscene.h>

Sprite::Sprite(QObject* parent): QObject(parent), QGraphicsItem(){

    currentFrame = 0;
    spriteImage = new QPixmap(":assets/HGSS/PokGen1.png");

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &Sprite::nextFrame);
    timer->start(25); //ms
}

QRectF Sprite::boundingRect() const{
    return QRectF(-10, -10, 20, 20);
}

void Sprite::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
   painter->drawPixmap(-10, -10, *spriteImage, currentFrame, 0, 20, 20);
   Q_UNUSED(option);
   Q_UNUSED(widget);
}




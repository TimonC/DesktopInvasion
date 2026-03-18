#include "Sprite.h"
#include <qglobal.h>
#include <qgraphicsscene.h>
#include <QString>

Sprite::Sprite(QObject* parent): QObject(parent), QGraphicsItem(){
    QString spritesheetPath = "assets/HGSS/PokGen1.png";
    spriteImage = new QPixmap(spritesheetPath);
    if (spriteImage->isNull()) {
        qWarning() << "Failed to load sprite image at " << spritesheetPath;
    }

    currentFrame = 0;
    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &Sprite::nextFrame);
    timer->start(25); //ms
}

QRectF Sprite::boundingRect() const{
    return QRectF(-10, -10, 20, 20);
}

QVariant Sprite::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneChange && scene()) {
        // scene() is the QGraphicsScene the item belongs to
        QRectF rect = scene()->sceneRect();
        setPos(rect.width()/2, rect.height()/2); // center in scene
    }
    return QGraphicsItem::itemChange(change, value);
}

void Sprite::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
   painter->drawPixmap(-10, -10, *spriteImage, currentFrame, 0, 20, 20);
   Q_UNUSED(option);
   Q_UNUSED(widget);
}
void Sprite::nextFrame() {
    // Advance to next frame horizontally
    currentFrame += 20;
    if (currentFrame >= spriteImage->width())
        currentFrame = 0;

    update(); // repaint this QGraphicsItem
}



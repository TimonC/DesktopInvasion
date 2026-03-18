#ifndef INVASIONVIEW_H
#define INVASIONVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QApplication>
#include <QRect>
#include <QRectF>
#include <QScreen>
#include <QGraphicsScene>


class InvasionView : public QGraphicsView
{
    Q_OBJECT
    public:
        InvasionView();
        const QGraphicsScene* getScene() const { return scene; }
        void addItem(QGraphicsItem* item) { scene->addItem(item); }
    private:
        QGraphicsScene *scene;
        void init();
};

#endif

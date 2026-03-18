#ifndef INVASIONVIEW_H
#define INVASIONVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>

class InvasionView : public QGraphicsView
{
    Q_OBJECT
    public:
        InvasionView();
    private:
        QGraphicsScene *scene;
        void init();
};

#endif

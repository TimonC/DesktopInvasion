#ifndef INVASIONVIEW_HH
#define INVASIONVIEW_HH

#include <QGraphicsView>
#include <QGraphicsScene>

class InvasionView : public QGraphicsView
{
    Q_OBJECT

    public:
        InvasionView;
    private:
        QGraphicsScene *scene;
};

#endif

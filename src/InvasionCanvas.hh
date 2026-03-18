#ifndef INVASIONVIEW_hh
#define INVASIONVIEW_hh

#include <QGraphicsView>
#include <QGraphicsScene>

class InvasionView : public QGraphicsView
{
    Q_OBJECT

public:
    InvasionView();

private:
    QGraphicsScene* scene;
};

#endif

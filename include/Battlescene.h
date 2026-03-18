#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include <QQuickView>
#include <Pokemon.h>

class Battlescene : QQuickView{
    Q_OBJECT

    public:
        explicit Battlescene(Pokemon *opponent, Pokemon *chosen, QWindow* parent = nullptr);

    private:
            int m_direction;
            QPoint m_origin;
};

#endif

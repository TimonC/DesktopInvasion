#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include <QQuickView>
#include <Pokemon.h>

class Battlescene : public QQuickView{
    Q_OBJECT

    public:
        explicit Battlescene(Pokemon *opponent, Pokemon *chosen, QWindow* parent = nullptr);
        void updateTextbar(std::string text);
    private:
            int m_direction;
            QPoint m_origin;
};

#endif

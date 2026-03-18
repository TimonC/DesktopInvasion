#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include <QQuickView>
#include <Pokemon.h>
#include <qtmetamacros.h>

class Battlescene : public QQuickView{
    Q_OBJECT

    public:
        explicit Battlescene(Pokemon *opponent, Pokemon *chosen, QWindow* parent = nullptr);
    signals:
        QString updateTextbar(const QString &text);
    private:
            int m_direction;
            QPoint m_origin;
};

#endif

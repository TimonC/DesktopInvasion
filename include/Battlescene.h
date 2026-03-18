#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include <QQuickView>
#include <Pokemon.h>
#include <qtmetamacros.h>

class Battlescene : public QQuickView
{
    Q_OBJECT

public:
    Battlescene(Pokemon *opp, Pokemon *chosen, QWindow *parent = nullptr);

    void updateTextbar(const std::string& text); // Make sure this signal exists

private:
    QPoint m_origin;
    int m_direction;
    QQuickItem *m_ui = nullptr;
};

#endif

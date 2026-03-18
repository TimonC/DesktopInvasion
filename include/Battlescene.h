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

private slots:
    void systemMove();
private:
    Pokemon* m_opp;
    Pokemon* m_chosen;
    QPoint m_origin;
    int m_direction;
    QQuickItem *m_ui = nullptr;
};

#endif

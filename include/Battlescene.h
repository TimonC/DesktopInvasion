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
    void run();
    void attack();
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent*  event) override;
private:
    bool m_dragging = false;
    void drag(QPoint delta);
    Pokemon* m_chosen;
    Pokemon* m_opp;
    QPoint m_origin;
    QPoint m_oldpos;
    int m_direction;
    QQuickItem *m_ui = nullptr;
};

#endif

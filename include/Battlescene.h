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
    void updateTextbar(const std::string& text);
    QQuickView* initCorners();
    QQuickView *m_corners = nullptr;

private slots:
    void run();
    void attack();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void initPosition();
    void drag(QPoint& delta);

    // Store initial positions for perfect sync
    QPoint m_initialBattlescenePos;
    QPoint m_initialCornersPos;
    QPoint m_initialOppPos;
    QPoint m_initialChosenPos;

    QPoint m_cornerSize;
    bool m_dragging = false;
    Pokemon* m_chosen;
    Pokemon* m_opp;
    QPoint m_origin;
    QPointF m_oldpos;
    int m_direction;
    QQuickItem *m_ui = nullptr;
};

#endif

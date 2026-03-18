#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include "DesktopScene.h"
#include <QQuickView>
#include <WildPokemon.h>
#include <qtmetamacros.h>

class Battlescene : public DesktopScene
{
    Q_OBJECT

public:
    Battlescene(WildPokemon *opp, WildPokemon *chosen, QWindow *parent = nullptr);
    void updateTextbar(const std::string& text);
    QQuickView* initCorners();
    QQuickView *m_corners = nullptr;

private slots:
    void handleDrag(bool isDragged) override;
    /* void run(); */
    /* void attack(); */


private:
    /* void initPosition(); */

    // Store initial positions for perfect sync
    QPoint m_initialBattlescenePos;
    QPoint m_initialCornersPos;
    QPoint m_initialOppPos;
    QPoint m_initialChosenPos;

    QPoint m_cornerSize;
    /* Pokemon* m_chosen; */
    /* Pokemon* m_opp; */
    QPoint m_origin;
    QQuickItem *m_ui = nullptr;
};

#endif

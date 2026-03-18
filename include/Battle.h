#ifndef BATTLE_H
#define BATTLE_H

#include "DesktopScene.h"
#include "pokemon_data.h"
#include <QQuickView>
#include <WildPokemon.h>
#include <qquickitem.h>
#include <qtmetamacros.h>
#include <qvariant.h>

class Battle : public DesktopScene
{
    Q_OBJECT

public:
    explicit Battle(int opp_direction, QPoint opp_pos, const PokemonInfo* opp, const PokemonInfo* chosen, QWindow *parent = nullptr);
    void updateTextbar(const std::string& text);
    QQuickView* initCorners();
    QQuickView *m_corners = nullptr;
    void direction(int direction) override;
private slots:
    void onBattleSceneLoaded(QVariant battleSceneItem);
    void handleDrag(bool isDragged) override;
    /* void run(); */
    /* void attack(); */

private:
    int m_pokeMargin = 4;
    void initPosition();
    // Store initial positions for perfect sync
    QPoint m_initialOppPos;

    void setupPokemon(const PokemonInfo* info, const char* role = "opponent");
    QQuickItem* m_battleScene;
    QQuickItem* m_opp;
    QQuickItem* m_chosen;
    const PokemonInfo* m_chosen_info;
    const PokemonInfo* m_opp_info;
    QPoint m_origin;
    QQuickItem *m_ui = nullptr;
};

#endif

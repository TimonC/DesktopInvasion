#ifndef BATTLE_H
#define BATTLE_H

#include <DesktopScene.h>
#include <pokemon_data.h>
#include <QQuickView>
#include <QQuickItem>
#include <WildPokemon.h>

class Battle : public DesktopScene
{
    Q_OBJECT

public:
    explicit Battle(WildPokemon* opp, const PokemonInfo* chosen_info, QWindow *parent = nullptr);
    void updateTextbar(const std::string& text);
    QQuickView* initCorners();
    QQuickView *m_corners = nullptr;
    void direction(int direction) override;

signals:
    void removeWildPokemon(const PokemonInfo* info);
private slots:
    void handleDrag(bool isDragged) override;
    void resetOpp(WildPokemon* opp);
private:

    template<typename Signal, typename Slot>
    inline void connectWithQML(Signal signal, Slot slot) {
        QObject* helper = new QObject(m_battleScene);
        QObject::connect(m_battleScene, signal, helper, SLOT(deleteLater()));
        QObject::connect(helper, &QObject::destroyed, slot);
    }

    int m_pokeMargin = 2;
    void initPosition();
    // Store initial positions for perfect sync
    QPoint m_initialOppPos;

    QQuickItem* setupPokemon(const PokemonInfo* info, const char* role = "opponent");
    QQuickItem* m_battleScene;
    QQuickItem* m_opp;
    QQuickItem* m_chosen;
    const PokemonInfo* m_chosen_info;
    const PokemonInfo* m_opp_info;
    QPoint m_origin;
    QQuickItem *m_ui = nullptr;
};

#endif

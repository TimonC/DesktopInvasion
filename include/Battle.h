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
    ~Battle() {
        qDebug() << "Battle destructor called!";
    }

    void updateTextbar(const std::string& text);
    QQuickView* initCorners();
    QQuickView *m_corners = nullptr;
    void direction(int direction) override;
    QPoint m_origin;

signals:
    void battleEnded(const char* endState);

public slots:
    void handleDrag(bool isDragged) override;

private:
    int m_pokeMargin = 2;
    void initPosition();

    WildPokemon* m_oppReference = nullptr;

    // Store initial positions for perfect sync
    QPoint m_initialOppPos;

    QQuickItem* setupPokemon(const PokemonInfo* info, const char* role = "opponent");
    QQuickItem* m_battleScene;
    QQuickItem* m_opp;
    QQuickItem* m_chosen;
    const PokemonInfo* m_chosen_info;
    const PokemonInfo* m_opp_info;
    QQuickItem *m_ui = nullptr;
};
#endif

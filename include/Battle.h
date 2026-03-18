#ifndef BATTLE_H
#define BATTLE_H
#include <DesktopScene.h>
#include <pokemon_data.h>
#include <QQuickView>
#include <QQuickItem>
#include <WildPokemon.h>
#include <qtmetamacros.h>

class Battle : public DesktopScene{

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
    void setupParty(Party party);
    void setSceneVisibility(bool visibility){
        m_battleScene->setProperty("visible",visibility);
    };

signals:
    void battleEnded(const char* endState);

public slots:
    void handleDrag(bool isDragged) override;
private slots:
    QQuickItem* updateSprite(int pokedexId, int generation, const char* role = "player");
    void handleSwitchedPokemon(int generation, int spriteId) {
        updateSprite(spriteId, generation, "player");
    }

private:
    int m_pokeMargin = 2;
    void initPosition();

    WildPokemon* m_oppReference = nullptr;

    // Store initial positions position swap with WildPokemon
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

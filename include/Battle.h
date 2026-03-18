#ifndef BATTLE_H
#define BATTLE_H
#include <DesktopScene.h>
#include <data_poke_asset.h>
#include <QQuickView>
#include <QQuickItem>
#include <WildPokemon.h>
#include <qglobal.h>
#include <qtmetamacros.h>
#include <PokeTypes.h>
#include <BattleMoveHandler.h>
#include <PokemonDatabase.h>

struct BattleMove{ //stub struct to build the move menu
    std::string name = "";
    std::string type = "Null";
};
struct Party {
    std::array<int, 6> pokedexIds{-1, -1, -1, -1, -1, -1};
    std::array<int, 6> spriteIds{-1, -1, -1, -1, -1, -1};
    std::array<int, 6> ballIds{-1, -1, -1, -1, -1, -1};
    std::array<std::string, 6> names{"", "", "", "", "", ""};
    std::array<int, 6> lvls{-1, -1, -1, -1, -1, -1};
    std::array<int, 6> healthTotals{-1, -1, -1, -1, -1, -1};
    std::array<std::array<BattleMove, 4>, 6> moves{};
};

class Battle : public DesktopScene{
    Q_OBJECT
    Q_DISABLE_COPY(Battle)

public:
    explicit Battle(QPoint initialOppPos, int initialOppDirection, PokemonState wildState, Party party, std::unique_ptr<BattleMoveHandler> battleMoveHandler, QWindow *parent = nullptr);
    ~Battle() {
        qDebug() << "Battle destructor called!";
    }

    void updateTextbar(const std::string& text);
    QQuickView* initCorners();
    QQuickView *m_corners = nullptr;
    void direction(int direction) override;

    // Store initial positions position swap with WildPokemon
    QPoint m_oppPos;
    QPoint m_spriteOffset;


    void setSceneVisibility(bool visibility){
        m_battleScene->setProperty("visible",visibility);
    };

    template<typename T>
    T getQMLSceneProperty(const char* property, const T& defaultValue = T()) {
        if (!m_battleScene || !property || !*property) return defaultValue;

        QVariant value = m_battleScene->property(property);
        return value.isValid() && value.canConvert<T>() ?
               value.value<T>() : defaultValue;
    }

    void showXPAndEndBattle(std::array<int,6> spread, std::array<int,6> lvlups);

signals:
    void battleEnded(const char* endState, bool removeWild);
    void _updatePartyXP(std::array<int, 6> spread);

public slots:
    void handleDrag(bool isDragged) override;

private slots:
    void handleBattleEnded(QString endState, bool removeWild);
    void handleGettingExperience();
    QQuickItem* updateSprite(int pokedexId, const char* role);
    void handleSwitchedPokemon(int partyIndex, int spriteId);
    void executeActionSequence(QVariantList sequence);


private:
    void setupParty(Party party);
    std::unique_ptr<BattleMoveHandler> m_battleMoveHandler;
    int m_pokeMargin = 2;
    void initPosition();


    QQuickItem* setupPokemon(int pokedexId, std::string name, int level, const char* role = "opponent");
    QQuickItem* m_battleScene;
    QQuickItem* m_opp;
    QQuickItem* m_chosen;
    const AssetInfo* m_chosen_info;
    const AssetInfo* m_opp_info;
    QQuickItem *m_ui = nullptr;
};
#endif

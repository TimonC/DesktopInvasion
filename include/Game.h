#ifndef GAME_H
#define GAME_H

#include "data_poke_asset.h"
#include "PokemonDatabase.h"
#include <Battle.h>
#include <GameMenu.h>
#include <WildPokemon.h>
#include <QObject>
#include <QQmlApplicationEngine>
#include <SystemTrayIcon.h>
#include <QTimer>
#include <random>

class Game : public QObject{
    Q_OBJECT

public:
    Game(QQmlApplicationEngine* engine, QWindow* parent = nullptr);
    ~Game();

public slots:
    void requestExit() {
        qDebug() << "Game exit requested";
        deleteLater();
    }
private:
    std::mt19937 m_rng;
    Party m_cachedParty;
    bool m_partyDirty = true;

    void fillPartySlot(Party& party, int slot, const PokemonState& pokemon);
    Party createPartyFromStates(const std::array<PokemonState, 6>& partyStates);
    void updatePartyCache();

    bool m_gameUsedToBeActive;
    QQmlApplicationEngine* m_engine = nullptr;
    GameMenu* m_menu;
    SystemTrayIcon* m_trayIcon;

    std::array<int, 6> m_partyIds;
    PokemonDatabase& m_db = PokemonDatabase::instance();


    WildPokemon* m_wildPokemon = nullptr;
    Battle* m_activeBattle = nullptr;
    QPoint m_spawnPoint = QPoint(-1, -1);
    int m_spawnDirection = -1;

    QTimer* m_spawnTimer;
    const int m_spawnDelay_ms = 1000;

    void initializeGame();
    void createInitialPokemon();
    void loadParty();
    const AssetInfo* getPartyPokemonInfo(int slot) const;
    void spawnPokemon();

    Party getParty();
    void updateWildPokemonPosToBattlePos();
    void safelyRemoveBattleScene();
    void safelyRemoveWildPokemon();

private slots:
    void updatePartyXP(std::array<int, 6> spread);
    void handleMenuOpen();
    void handleMenuClosed();
    void handleBattleStart();
    void handleBattleEnd(const char* endState, bool removeWild);
    void setGameActive(bool active = true);
};

#endif

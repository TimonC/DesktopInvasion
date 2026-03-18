#ifndef GAME_H
#define GAME_H

#include "data_poke.h"
#include "PokemonDatabase.h"
#include <Battle.h>
#include <GameMenu.h>
#include <WildPokemon.h>
#include <QObject>
#include <QQmlApplicationEngine>
#include <SystemTrayIcon.h>
#include <QTimer>

class Game : public QObject{
    Q_OBJECT

public:
    Game(QQmlApplicationEngine* engine, QWindow* parent = nullptr);
    ~Game();

private:
    bool m_gameUsedToBeActive;
    QQmlApplicationEngine* m_engine = nullptr;
    GameMenu* m_menu;
    SystemTrayIcon* m_trayIcon;

    std::array<int, 6> m_partyIds;  // Database IDs of party Pokemon
    PokemonDatabase& m_db = PokemonDatabase::instance();

    WildPokemon* m_wildPokemon = nullptr;
    Battle* m_activeBattle = nullptr;
    const PokemonInfo* m_wildPokemonInfo = nullptr;
    QPoint m_spawnPoint = QPoint(-1, -1);
    int m_spawnDirection = -1;

    QTimer* m_spawnTimer;
    const int m_spawnDelay_ms = 1000;

    void initializeGame();
    void createInitialPokemon();
    void loadParty();
    const PokemonInfo* getPartyPokemonInfo(int slot) const;
    void spawnPokemon();

    Party getParty();
    void updateWildPokemonPosToBattlePos();
    void safelyRemoveBattleScene();

    Poke initBattleState(int uid);

private slots:
    void handleMenuOpen();
    void handleMenuClosed();
    void handleBattleStart();
    void handleBattleEnd(const char* endState);
    void setGameActive(bool active = true);
};

#endif

#ifndef GAME_H
#define GAME_H

#include "PokemonDatabase.h"
#include <Battle.h>
#include <GameMenu.h>
#include <WildPokemon.h>
#include <QObject>
#include <QQmlApplicationEngine>
#include <SystemTrayIcon.h>
#include <QTimer>
#include <QVariantList>
#include <random>

class Game : public QObject {
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
    std::mt19937             m_rng;
    bool                     m_gameUsedToBeActive;
    QQmlApplicationEngine*   m_engine      = nullptr;
    GameMenu*                m_menu;
    SystemTrayIcon*          m_trayIcon;
    PokemonDatabase&         m_db          = PokemonDatabase::instance();
    WildPokemon*             m_wildPokemon = nullptr;
    Battle*                  m_activeBattle= nullptr;
    QPoint                   m_spawnPoint  = QPoint(-1,-1);
    int                      m_spawnDirection = -1;
    QTimer*                  m_spawnTimer;
    const int                m_spawnDelay_ms = 1000;

    void initializeGame();
    void createInitialPokemon();
    void spawnPokemon();
    void safelyRemoveBattleScene();
    void safelyRemoveWildPokemon();

    // Menu <-> DB bridge helpers
    QVariantMap pokemonToMenuState(int slot, const PokemonState& p);
    QVariantList partyToVariantList();
    QVariantList boxToVariantList(int boxIndex);
    void         pushBoxToMenu(int boxIndex);

private slots:
    void updatePartyXP(std::array<int,6> spread);
    void handleMenuOpen();
    void handleMenuClosed();
    void handleMenuPreloadBox(int boxIndex);
    void handleBattleStart();
    void handleBattleEnd(const char* endState, bool removeWild);
    void setGameActive(bool active = true);
};

#endif

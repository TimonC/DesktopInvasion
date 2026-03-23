#ifndef GAME_H
#define GAME_H

#include "PokemonDatabase.h"
#include <Battle.h>
#include <GameMenu.h>
#include <WildPokemon.h>
#include <QObject>
#include <SystemTrayIcon.h>
#include <QTimer>
#include <QVariantList>
#include <random>

class Game : public QObject {
    Q_OBJECT
public:
    Game(QWindow* parent = nullptr);
    ~Game();

public slots:
    void requestExit() {
        qDebug() << "Game exit requested";
        deleteLater();
    }


private:
    void resetGame();
    void openStarterMenu();
    void writeDefaults();
    void initMenu();
    std::mt19937             m_rng;
    bool                     m_gameUsedToBeActive = true;
    GameMenu*                m_menu        = nullptr;
    QQuickView*              m_starterMenu        = nullptr;
    SystemTrayIcon*          m_trayIcon;
    PokemonDatabase&         m_db          = PokemonDatabase::instance();
    WildPokemon*             m_wildPokemon = nullptr;
    Battle*                  m_activeBattle= nullptr;
    QPoint                   m_spawnPoint  = QPoint(-1,-1);
    int                      m_spawnDirection = -1;
    QTimer*                  m_spawnTimer;
    const int                m_spawnDelay_ms = 1000;

    void initializeGame(bool openStarter = true);
    void spawnPokemon();
    void safelyRemoveBattleScene();
    void safelyRemoveWildPokemon();

    // Menu <-> DB bridge helpers
    QVariantMap pokemonToMenuState(int slot, const PokemonState& p);
    QVariantList partyToVariantList();
    QVariantList boxToVariantList(int boxIndex);
    void         pushBoxToMenu(int boxIndex);


private slots:
    void handleSaveSelected(int saveId);
    void onStarterMenuFinished(QString playerName, int trainerId, int starterPokedexId);
    void updatePartyXP(std::array<int,6> spread);
    void handleMenuOpen();
    void handleMenuClosed();
    void handleMenuPreloadBox(int boxIndex);
    void handlePCSwap(int xplace, int xpos, int yplace, int ypos);
    void handleNameChange(int xplace, int xpos, QString name);
    void handleMoveChange(int xplace, int xpos, int moveSlot, int moveId);
    void handleBattleStart();
    void handleBattleEnd(const char* endState, bool removeWild);
    void setGameActive(bool active = true);
};

#endif

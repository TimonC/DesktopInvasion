#ifndef GAME_H
#define GAME_H
#include "pokemon_data.h"
#include <Battle.h>
#include <GameMenu.h>
#include <WildPokemon.h>
#include <QObject>
#include <QQmlApplicationEngine>
#include <SystemTrayIcon.h>

class Game : public QObject{
    Q_OBJECT
public:
    Game(QQmlApplicationEngine* engine, QObject* parent = nullptr);
    ~Game();
private:
    GameMenu* m_menu;
    SystemTrayIcon* m_trayIcon;
    WildPokemon* m_wildPokemon = nullptr;
    Battle* m_activeBattle = nullptr;
    const int m_spawnDelay_ms = 5000;

private slots:
    void spawnWildPokemon(const PokemonInfo* info);
    void handleBattleStart(Battle* battle);
    void handleBattleEnd(Battle* battle, WildPokemon* wild, bool removeWild);
    void setSpawnActive(bool active = true);
    void setGameActive(bool active = true);
};

#endif

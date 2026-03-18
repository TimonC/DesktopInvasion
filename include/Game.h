#ifndef GAME_H
#define GAME_H
#include "pokemon_data.h"
#include <Battle.h>
#include <GameMenu.h>
#include <WildPokemon.h>
#include <QObject>
#include <QQmlApplicationEngine>
#include <SystemTrayIcon.h>
#include <qqmlapplicationengine.h>

class Game : public QObject{
    Q_OBJECT

public:
    Game(QQmlApplicationEngine* engine, QObject* parent = nullptr);
    ~Game();

private:
    QQmlApplicationEngine* m_engine  =  nullptr;
    WildPokemon* m_wildPokemon = nullptr;
    Battle* m_activeBattle = nullptr;
    const PokemonInfo* m_wildPokemonInfo;
    GameMenu* m_menu;
    SystemTrayIcon* m_trayIcon;
    QPoint m_spawnPoint = QPoint(-1,-1);
    int m_spawnDirection = -1;
    const int m_spawnDelay_ms = 5000;
    void updateWildPokemonPosToBattlePos();
    void spawnPokemon();

private slots:
    void handleBattleStart(Battle* battle);
    void handleBattleEnd(bool removeWild);
    void setGameActive(bool active = true);
};

#endif

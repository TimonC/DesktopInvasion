#ifndef GAME_H
#define GAME_H

#include "pokemon_data.h"
#include <Battle.h>
#include <GameMenu.h>
#include <WildPokemon.h>
#include <QObject>
#include <vector>
#include <utility>

class Game : public QObject{
    Q_OBJECT

public:
    Game(QObject* parent = nullptr);
    ~Game();

    void enableSpawn(bool enable = true);

private:
    GameMenu* m_menu;
    const static int MAX_WILD_SPAWNS = 20;
    WildPokemon* m_wildSpawns[MAX_WILD_SPAWNS];
    int m_activeSpawnCount = 0;

    std::vector<std::pair<WildPokemon*, Battle*>> m_wildBattlePairs;

private slots:
    void pushWildPokemon(const PokemonInfo* info);
    void popWildPokemon();
    void handleBattleStart(Battle* battle);
};

#endif

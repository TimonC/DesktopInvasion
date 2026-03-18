#ifndef GAME_H
#define GAME_H

#include "pokemon_data.h"
#include <Battle.h>
#include <GameMenu.h>
#include <WildPokemon.h>
#include <QObject>
#include <vector>

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

    // Each Battle must have a WildPokemon, but not vice versa
    struct BattleEntry {
        Battle* battle;
        WildPokemon* wild;
        QMetaObject::Connection connection;  // Store the connection

        BattleEntry(Battle* b, WildPokemon* w) : battle(b), wild(w) {}
    };

    std::vector<BattleEntry> m_battles;

private slots:
    void pushWildPokemon(const PokemonInfo* info);
    void popWildPokemon();
    void handleBattleStart(Battle* battle);
    void cleanupBattle(Battle* battle);  // When battle ends
};

#endif

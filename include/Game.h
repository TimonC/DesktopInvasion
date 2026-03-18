#ifndef GAME_H
#define GAME_H

#include "pokemon_data.h"
#include <GameMenu.h>
#include <iostream>
#include <WildPokemon.h>

class Game : public QObject{
    Q_OBJECT

public:
    Game(QObject* parent = nullptr);
    ~Game(){
        std::cout << "Deleting game!\n";
        delete m_menu;
    }

    void enableSpawn(bool enable = true);

private:
    GameMenu* m_menu;
    const static int MAX_WILD_SPAWNS = 20;
    WildPokemon* m_wildSpawns[MAX_WILD_SPAWNS]; //Good ol' raw pointers cause I'm making this game as a learning exercise after all
    int m_activeSpawnCount = 0; //This might always be 0 or 1 since I'm not sure yet if I want to have concurrent WildPokemon
private slots:
    void pushWildPokemon(const PokemonInfo* info);
    void popWildPokemon();
};

#endif

#include <Game.h>
#include <WildPokemon.h>
#include <QOpenGLContext>
#include <QLoggingCategory>

Game::Game(){
    m_menu = new GameMenu();
}

void Game::enableSpawn(bool enable){

}


void Game::spawnWildPokemon(const PokemonInfo* info){
    m_wildSpawns[m_activeSpawnCount] = new WildPokemon(info);
    m_activeSpawnCount+=1;
}

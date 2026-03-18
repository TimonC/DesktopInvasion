#include <Game.h>
#include <WildPokemon.h>
#include <QOpenGLContext>
#include <QLoggingCategory>
#include <globals.h>

Game::Game(){
    m_menu = new GameMenu();
    pushWildPokemon(Globals::getPokemonInfo());
}

void Game::enableSpawn(bool enable){

}

void Game::pushWildPokemon(const PokemonInfo* info){
    if (m_activeSpawnCount < MAX_WILD_SPAWNS) {
        m_wildSpawns[m_activeSpawnCount] = new WildPokemon(info);
        m_activeSpawnCount++;
    }
}

void Game::popWildPokemon(){
    if (m_activeSpawnCount > 0) {
        m_activeSpawnCount--;
        delete m_wildSpawns[m_activeSpawnCount];
        m_wildSpawns[m_activeSpawnCount] = nullptr;
    }
}

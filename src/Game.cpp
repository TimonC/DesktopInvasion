#include <Game.h>
#include <WildPokemon.h>
#include <QOpenGLContext>
#include <QLoggingCategory>
#include <globals.h>
#include <Player.h>

Game::Game(QObject* parent) : QObject(parent){
    m_menu = new GameMenu();
    pushWildPokemon(Globals::getPokemonInfo());

    connect(&Globals::getPlayer(), SIGNAL(startABattle(Battle*)),
            this, SLOT(handleBattleStart(Battle*)));
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

void Game::handleBattleStart(Battle* battle) {
    if (m_activeSpawnCount > 0) {
        WildPokemon* lastWild = m_wildSpawns[m_activeSpawnCount - 1];
        m_wildBattlePairs.push_back(std::make_pair(lastWild, battle));
    }
}

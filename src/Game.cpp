#include <Game.h>
#include <WildPokemon.h>
#include <QOpenGLContext>
#include <QLoggingCategory>
#include <globals.h>
#include <Player.h>

Game::Game(QObject* parent) : QObject(parent){
    m_menu = new GameMenu();
    // Initialize all pointers to nullptr
    for (int i = 0; i < MAX_WILD_SPAWNS; ++i) {
        m_wildSpawns[i] = nullptr;
    }

    pushWildPokemon(Globals::getPokemonInfo());

    connect(&Globals::getPlayer(), &Player::startABattle,
            this, &Game::handleBattleStart);
}

Game::~Game() {
    qDebug("Deleting game!\n");

    for (auto& pair : m_wildBattlePairs) {
        delete pair.second;  // Delete Battle*
    }
    m_wildBattlePairs.clear();

    for (int i = 0; i < m_activeSpawnCount; ++i) {
        delete m_wildSpawns[i];
        m_wildSpawns[i] = nullptr;
    }
    m_activeSpawnCount = 0;

    delete m_menu;
    m_menu = nullptr;
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

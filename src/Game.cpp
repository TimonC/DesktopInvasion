#include <Game.h>
#include <WildPokemon.h>
#include <QOpenGLContext>
#include <QLoggingCategory>
#include <globals.h>
#include <Player.h>

Game::Game(QObject* parent) : QObject(parent){
    m_menu = new GameMenu();
    for (int i = 0; i < MAX_WILD_SPAWNS; ++i) {
        m_wildSpawns[i] = nullptr;
    }

    pushWildPokemon(Globals::getPokemonInfo());
    connect(&Globals::getPlayer(), &Player::startABattle,
            this, &Game::handleBattleStart);
}

Game::~Game() {
    qDebug("Deleting game!\n");

    // Clean up all battles (automatically disconnects connections)
    for (auto& entry : m_battles) {
        delete entry.battle;
    }
    m_battles.clear();

    // Clean up wild pokemon
    for (int i = 0; i < m_activeSpawnCount; ++i) {
        delete m_wildSpawns[i];
        m_wildSpawns[i] = nullptr;
    }
    m_activeSpawnCount = 0;

    delete m_menu;
    m_menu = nullptr;
}

void Game::enableSpawn(bool enable) {
    // Implementation
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
        WildPokemon* wildToRemove = m_wildSpawns[m_activeSpawnCount];

        // Remove any battle associated with this wild pokemon
        for (auto it = m_battles.begin(); it != m_battles.end(); ++it) {
            if (it->wild == wildToRemove) {
                delete it->battle;
                m_battles.erase(it);
                break;
            }
        }

        delete wildToRemove;
        m_wildSpawns[m_activeSpawnCount] = nullptr;
    }
}

void Game::handleBattleStart(Battle* battle) {
    if (m_activeSpawnCount > 0) {
        WildPokemon* lastWild = m_wildSpawns[m_activeSpawnCount - 1];

        // Connect battle's ended signal to cleanup
        QMetaObject::Connection conn = connect(
            battle, &Battle::destroyed,  // Or custom 'battleEnded' signal
            this, [this, battle]() { cleanupBattle(battle); }
        );

        // Store battle with its wild pokemon and connection
        m_battles.emplace_back(battle, lastWild);
        m_battles.back().connection = conn;  // Store the connection
    }
}

void Game::cleanupBattle(Battle* battle) {
    for (auto it = m_battles.begin(); it != m_battles.end(); ++it) {
        if (it->battle == battle) {
            // Disconnect automatically when entry is destroyed
            m_battles.erase(it);
            break;
        }
    }
}

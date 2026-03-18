#include <Game.h>
#include <WildPokemon.h>
#include <globals.h>
#include <Player.h>
#include <QTimer>
#include <utils/connectWithQML.h>

Game::Game(QQmlApplicationEngine* engine, QObject* parent) : QObject(parent){

    m_menu = new GameMenu();

    spawnWildPokemon(Globals::getPokemonInfo());
    connect(&Globals::getPlayer(), &Player::startABattle,
            this, &Game::handleBattleStart);

    QObject* systemTrayIcon = engine->rootObjects()[0];
    connectWithQML(systemTrayIcon, SIGNAL(gameActive()), [this](){
                setGameActive();
            });
}

Game::~Game() {
    if (m_activeBattle) {
        m_activeBattle->deleteLater();
    }
    if (m_wildPokemon) {
        m_wildPokemon->deleteLater();
    }
    delete m_menu;
}

void Game::setSpawnActive(bool active) {
    // Implementation
}

void Game::setGameActive(bool active){
    if(active){
        m_activeBattle->close();
        m_wildPokemon->close();
    }else{
        m_activeBattle->show();
        m_wildPokemon->show();
    }
}

void Game::spawnWildPokemon(const PokemonInfo* info){
    assert(!m_wildPokemon && "Cannot spawn: WildPokemon already exists");
    m_wildPokemon = new WildPokemon(info);
}

void Game::handleBattleStart(Battle* battle) {
    assert(m_wildPokemon && "Cannot start battle: No WildPokemon exists");
    assert(!m_activeBattle && "Cannot start battle: Battle already active");

    m_activeBattle = battle;
    connect(battle, &Battle::battleEnded,
            this, [this](Battle* b, WildPokemon* wild, bool removeWild) {
        handleBattleEnd(b, wild, removeWild);
    });
}

void Game::handleBattleEnd(Battle* battle, WildPokemon* opp, bool removeWild) {
    assert(battle == m_activeBattle && "Battle mismatch in handleBattleEnd");

    disconnect(battle, nullptr, this, nullptr);
        QTimer::singleShot(10, this, [this,battle]() {
            battle->setProperty("visible", false);
            battle->deleteLater();
            m_activeBattle = nullptr;
        });

    if (removeWild) {
        assert(m_wildPokemon == opp && "WildPokemon mismatch in handleBattleEnd");
        m_wildPokemon->deleteLater();
        m_wildPokemon = nullptr;
        QTimer::singleShot(m_spawnDelay_ms, this, [this]() {
            spawnWildPokemon(Globals::getPokemonInfo());
        });
    }
}

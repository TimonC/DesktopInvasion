#include <Game.h>
#include <WildPokemon.h>
#include <globals.h>
#include <Player.h>
#include <QTimer>

Game::Game(QObject* parent) : QObject(parent){
    m_menu = new GameMenu();
    spawnWildPokemon(Globals::getPokemonInfo());
    connect(&Globals::getPlayer(), &Player::startABattle,
            this, &Game::handleBattleStart);
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

void Game::enableSpawn(bool enable) {
    // Implementation
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
    m_activeBattle = nullptr;
    battle->deleteLater();

    if (removeWild) {
        assert(m_wildPokemon == opp && "WildPokemon mismatch in handleBattleEnd");
        m_wildPokemon->deleteLater();
        m_wildPokemon = nullptr;
        QTimer::singleShot(m_spawnDelay_ms, this, [this]() {
            spawnWildPokemon(Globals::getPokemonInfo());
        });
    }
}

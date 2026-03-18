#include <Game.h>
#include <WildPokemon.h>
#include <globals.h>
#include <Player.h>
#include <QTimer>

Game::Game(QQmlApplicationEngine* engine, QObject* parent) : QObject(parent) {
    m_menu = new GameMenu();
    m_engine  = engine;

    m_wildPokemonInfo = Globals::getPokemonInfo();
    m_wildPokemon = new WildPokemon(m_wildPokemonInfo);

    connect(&Globals::getPlayer(), &Player::startABattle,
            this, &Game::handleBattleStart);

    m_trayIcon = new SystemTrayIcon(this);
    connect(m_trayIcon, &SystemTrayIcon::gameActive,
            this, &Game::setGameActive);
}

Game::~Game() {
    if (m_activeBattle) {
        m_activeBattle->deleteLater();
    }
    if (m_wildPokemon) {
        m_wildPokemon->deleteLater();
    }
    delete m_menu;
    delete m_trayIcon;
}

void Game::setGameActive(bool active){
    static bool processing = false;
    if(processing) return;

    processing=true;
    if(active){
        if(m_wildPokemon){
            m_wildPokemon->disconnect();
            delete m_wildPokemon;
            m_wildPokemon = nullptr;
        }
        if(m_wildPokemonInfo)m_wildPokemon = new WildPokemon(m_wildPokemonInfo, m_spawnPoint, m_spawnDirection);
    }else{
        if(m_activeBattle){
            updateWildPokemonPosToBattlePos();
            m_activeBattle->disconnect();
            delete m_activeBattle;
            m_activeBattle = nullptr;
        }
        if(m_wildPokemon){
            m_spawnPoint = m_wildPokemon->position();
            m_spawnDirection = m_wildPokemon->m_currentDirection;
            m_wildPokemon->disconnect();
            delete m_wildPokemon;
            m_wildPokemon = nullptr;
        }
    }
    processing=false;
}

void Game::updateWildPokemonPosToBattlePos(){
        QPoint newOppPos = m_wildPokemon->position() + (m_activeBattle->position() - m_activeBattle->m_origin);
        m_wildPokemon->setPosition(newOppPos);
}

void Game::handleBattleStart(Battle* battle) {
    assert(m_wildPokemon && "Cannot start battle: No WildPokemon exists");
    assert(!m_activeBattle && "Cannot start battle: Battle already active");

    m_activeBattle = battle;
    connect(battle, &Battle::battleEnded,
            this, [this](bool removeWild) {
        handleBattleEnd(removeWild);
    });
}

void Game::handleBattleEnd(bool removeWild) {
    disconnect(m_activeBattle, nullptr, this, nullptr);

    if (removeWild) {
        m_activeBattle->deleteLater();
        m_activeBattle = nullptr;

        m_wildPokemon->deleteLater();
        m_wildPokemon = nullptr;

        //Delay until the new spawn
        QTimer::singleShot(m_spawnDelay_ms, this, [this]() {
            m_wildPokemonInfo = Globals::getPokemonInfo();
            m_wildPokemon = new WildPokemon(m_wildPokemonInfo);
        });
    }else{
        m_activeBattle->handleDrag(false);
        updateWildPokemonPosToBattlePos();
        m_wildPokemon->show();


        //Short delay to ensure smooth visual transition
        QTimer::singleShot(100, this, [this]() {
            m_activeBattle->deleteLater();
            m_activeBattle = nullptr;
            m_wildPokemon->roaming(true);
        });
    }
}

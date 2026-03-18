#include "BattleMoveHandler.h"
#include "SystemTrayIcon.h"
#include "Game.h"
#include "WildPokemon.h"
#include "globals.h"
#include <QTimer>
#include <QDebug>
#include <cstring>
#include <form_mapper.h>

Game::Game(QQmlApplicationEngine* engine, QWindow* parent)
    : QObject(parent)
    , m_engine(engine)
    , m_menu(new GameMenu())
    , m_trayIcon(new SystemTrayIcon(this))
    , m_spawnTimer(new QTimer(this))
{
    qDebug() << "Game constructor called!";

    m_gameUsedToBeActive = true;

    m_partyIds.fill(0);

    if (!m_db.initialize()) {
        qWarning() << "Failed to initialize database!";
    }

    initializeGame();

    connect(m_trayIcon, &SystemTrayIcon::gameActive, this, &Game::setGameActive);

    connect(m_trayIcon, &SystemTrayIcon::menuButtonPressed, this, &Game::handleMenuOpen);

    connect(m_menu, &GameMenu::menuClosed, this, &Game::handleMenuClosed);

    m_spawnTimer->setInterval(m_spawnDelay_ms);
    connect(m_spawnTimer, &QTimer::timeout, this, &Game::spawnPokemon);
    m_spawnTimer->start();
}

Game::~Game() {
    if (m_activeBattle) delete m_activeBattle;
    if (m_wildPokemon) delete m_wildPokemon;
    delete m_menu;
}

void Game::safelyRemoveBattleScene(){
    /* m_activeBattle->disconnect(); //this might not be secure with deleteLater, could potentially be the cause of freezes */
                                  //but i do it here to avoid a bug where all all the battleended signals are triggered
    disconnect(m_activeBattle, &Battle::battleEnded,
            this, &Game::handleBattleEnd); //Only disconnect the battle-game connection to avoid faulty trigger w/o freezes
    m_activeBattle->deleteLater();
    m_activeBattle = nullptr;
}

void Game::handleMenuOpen(){
    bool usedToBeActive = m_gameUsedToBeActive;
    setGameActive(false);
    m_gameUsedToBeActive = usedToBeActive;

    m_menu->activate();
    m_trayIcon->enabled(false);
}

void Game::handleMenuClosed(){
    m_trayIcon->enabled(true);
    if(m_gameUsedToBeActive) setGameActive(true);
};

void Game::setGameActive(bool active) {
    static bool processing = false;
    if (processing) return;

    m_gameUsedToBeActive = active;

    processing = true;

    if (active){
            // Singleshot to ensure we're in next event loop iteration
        QTimer::singleShot(0, this, &Game::spawnPokemon);
    }
    else{
        if (m_activeBattle) {
            m_activeBattle->setSceneVisibility(false);
            updateWildPokemonPosToBattlePos();
            safelyRemoveBattleScene();
        }
        if (m_wildPokemon) {
            m_spawnPoint = m_wildPokemon->position();
            m_spawnDirection = m_wildPokemon->m_currentDirection;
            m_wildPokemon->deleteLater();
            m_wildPokemon = nullptr;
        }
            //Wait for deletions to complete
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    processing = false;
}

void Game::initializeGame() {
    GameState state = m_db.loadGameState();

    bool hasParty = false;
    for (int i = 0; i < 6; i++) {
        if (state.party_id[i] > 0) {
            m_partyIds[i] = state.party_id[i];
            hasParty = true;
        }
    }

    if (!hasParty) {
        qDebug() << "New game detected, creating initial Pokemon...";
        createInitialPokemon();
        loadParty();
    } else {
        qDebug() << "Loading saved game from Player" << QString::fromStdString(state.name) << "...";
        loadParty();
    }

    PokemonState wildState = m_db.getWildPokemon();
    if (wildState.pokedex_id > 0) {
        m_wildPokemonInfo = Globals::getPokemonInfo(wildState.pokedex_id);
        qDebug() << "Found wild Pokemon in database:" << QString::fromStdString(wildState.name);
    }
}

void Game::createInitialPokemon() {
    PokemonState dusclops;
    dusclops.pokedex_id = 356;
    dusclops.name = "Dusclops";

    for (int i = 0; i < 6; i++) {
        dusclops.ivs[i] = 32;
        dusclops.evs[i] = 50;
    }

    dusclops.nature = Nature::Hardy;
    dusclops.total_xp = 0;

    int pokemonId = m_db.createPokemon(dusclops);
    if (pokemonId > 0) {
        qDebug() << "Created Dusclops with database ID:" << pokemonId;
        m_db.setPartyPokemon(0, pokemonId);
        m_partyIds[0] = pokemonId;
    }
}

void Game::loadParty() {
    GameState state = m_db.loadGameState();

    for (int i = 0; i < 6; i++) {
        m_partyIds[i] = state.party_id[i];
        if (m_partyIds[i] > 0) {
            PokemonState pokemon = m_db.getPokemon(m_partyIds[i]);
            if (pokemon._id > 0) {
                qDebug() << "Loaded Pokemon" << QString::fromStdString(pokemon.name)
                         << "to party slot" << i;
            }
        }
    }
}

const PokemonInfo* Game::getPartyPokemonInfo(int slot) const {
    if (slot < 0 || slot >= 6 || m_partyIds[slot] <= 0) {
        return nullptr;
    }

    PokemonState pokemon = m_db.getPokemon(m_partyIds[slot]);
    if (pokemon._id > 0) {
        return Globals::getPokemonInfo(pokemon.pokedex_id);
    }
    return nullptr;
}

void Game::spawnPokemon() {
    if (m_wildPokemon) {
        m_wildPokemon->deleteLater();
        m_wildPokemon = nullptr;
    }

    PokemonState wildState = m_db.getWildPokemon();
    if (wildState.pokedex_id > 0) {
        m_wildPokemonInfo = Globals::getPokemonInfo(wildState.pokedex_id);
        qDebug() << "Spawning existing wild Pokemon:" << QString::fromStdString(wildState.name);
    } else {
        m_wildPokemonInfo = Globals::getPokemonInfo(0);

        PokemonState newWild;
        newWild.pokedex_id = m_wildPokemonInfo->pokedexId;
        newWild.name = m_wildPokemonInfo->name;

        for (int i = 0; i < 6; i++) {
            newWild.ivs[i] = 32;
            newWild.evs[i] = 50;
        }

        newWild.nature = Nature::Hardy;
        newWild.total_xp = 0;

        m_db.spawnWildPokemon(newWild);
        qDebug() << "Created new wild Pokemon:" << QString::fromStdString(newWild.name);
    }

    if (m_wildPokemonInfo) {
        m_wildPokemon = new WildPokemon(m_wildPokemonInfo, m_spawnPoint, m_spawnDirection);

        connect(m_wildPokemon, &WildPokemon::startABattle, this, &Game::handleBattleStart);

        m_wildPokemon->show();
        m_spawnTimer->stop();
    }
}

Party Game::getParty() {
    GameState state = m_db.loadGameState();
    Party party;

    for(int i = 0; i < 6; ++i) {
        int pokemonId = state.party_id[i];
        if(pokemonId <= 0) continue;

        PokemonState pokemon = m_db.getPokemon(pokemonId);
        const PokemonInfo* info = Globals::getPokemonInfo(pokemon.pokedex_id);
        if(!info) continue;

        party.pokedexIds[i] = info->pokedexId;
        party.spriteIds[i] = info->spriteId;
        party.iconIds[i] = FormMapper::toIconId(pokemon.pokedex_id, 0);
        party.names[i] = pokemon.name;
        party.gens[i] = info->generation;
        party.ballIds[i] = 3;
    }

    return party;
}
void Game::handleBattleStart() {
    if (!m_wildPokemon || m_activeBattle) {
        qWarning() << "Cannot start battle - invalid state";
        return;
    }

    Poke partyBattleState[6];
    for(int i = 0; i<6; i++){
        partyBattleState[i] = initBattleState(m_partyIds[i]);
    };

    auto battleMoveHandler = std::make_unique<BattleMoveHandler>(initBattleState(0),partyBattleState);
    m_activeBattle = new Battle(m_wildPokemon, getParty(), std::move(battleMoveHandler));
    connect(m_activeBattle, &Battle::battleEnded,
            this, &Game::handleBattleEnd);

    qDebug() << "Starting battle...";
}

Poke Game::initBattleState(int uid){
    PokemonState dataState = m_db.getPokemon(uid);
    return
    {
        {
            uid,
            {100,100,100,100,100,100},
            {Type::Normal, Type::Null},
            { 1, -1, -1, -1},
        },
        {
            100,
            {}}
    };
};



void Game::handleBattleEnd(const char* endState) {
    if (!endState) {
        qWarning() << "handleBattleEnd called with null endState";
        return;
    }

    qDebug() << endState;
    bool playerWon = (strcmp(endState, "PlayerWon") == 0);
    bool opponentCaught = (strcmp(endState, "OpponentCaught") == 0);
    bool opponentWon = strcmp(endState, "OpponentWon") == 0;
    bool removeWild = playerWon || opponentCaught || opponentWon;

    if (removeWild) {
        // Handle catching
        if (opponentCaught) {
            int caughtId = m_db.catchWildPokemon();
            if (caughtId > 0) {
                PokemonState caughtPokemon = m_db.getPokemon(caughtId);
                QString caughtName = QString::fromStdString(caughtPokemon.name);

                qDebug() << caughtName << "caught! Database ID:" << caughtId;

                // Add to first empty party slot
                for (int i = 0; i < 6; i++) {
                    if (m_partyIds[i] == 0) {
                        m_db.setPartyPokemon(i, caughtId);
                        m_partyIds[i] = caughtId;

                        qDebug() << "Added" << caughtName
                                 << "to party slot" << i;
                        break;
                    }
                }
            }
        }

        // Player won the battle - add XP
        if (playerWon) {
            if (m_db.addPokemonXp(m_partyIds[0], 100)) {
                int newXp = m_db.getPokemonXp(m_partyIds[0]);
                qDebug() << "Added 100 XP to fighting Pokemon. Total XP:" << newXp;
            } else {
                qWarning() << "Failed to add XP to Pokemon ID:" << m_partyIds[0];
            }
        }

        // Cleanup wild pokemon and battle
        if( m_db.clearWild()){
            qDebug() << "Cleared wild pokemon instance";
        }else{
            qWarning() << "Failed to clear wild pokemon";
        }
        if (m_activeBattle)  safelyRemoveBattleScene();
        if (m_wildPokemon) {
            m_wildPokemon->deleteLater();
            m_wildPokemon = nullptr;
        }
        m_spawnPoint = QPoint(-1,-1);
        m_spawnTimer->start();

    } else {
        // Battle ended without removing wild Pokemon
        if (m_activeBattle) {
            m_activeBattle->handleDrag(false);
        }

        updateWildPokemonPosToBattlePos();

        if (m_wildPokemon) {
            m_wildPokemon->show();
        }

        //Cleanup with delay for smooth transition from battlescene to wild pokemon
        QTimer::singleShot(100, this, [this]() {
            if (m_activeBattle) safelyRemoveBattleScene();
            if (m_wildPokemon) m_wildPokemon->roaming(true);
        });
    }
}


void Game::updateWildPokemonPosToBattlePos() {
    if (m_wildPokemon && m_activeBattle) {
        QPoint newOppPos = m_wildPokemon->position() +
                          (m_activeBattle->position() - m_activeBattle->m_origin);
        m_wildPokemon->setPosition(newOppPos);
    }
}

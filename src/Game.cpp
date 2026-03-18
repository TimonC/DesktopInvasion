#include <Game.h>
#include <PokeTypes.h>
#include <SystemTrayIcon.h>
#include <WildPokemon.h>
#include <globals.h>
#include <PokeMath.h>
#include <QTimer>
#include <QDebug>
#include <cstring>
#include <BattleMoveHandler.h>
#include <data_poke_asset.h>
#include <lookup.h>
#include <qglobal.h>

Game::Game(QQmlApplicationEngine* engine, QWindow* parent)
    : QObject(parent)
    , m_engine(engine)
    , m_menu(new GameMenu())
    , m_trayIcon(new SystemTrayIcon(this))
    , m_spawnTimer(new QTimer(this))
    , m_rng(std::random_device{}())
{
    qDebug() << "Game constructor called!";
    m_gameUsedToBeActive = true;
    initializeGame();

    connect(m_trayIcon, &SystemTrayIcon::menuButtonPressed, this, &Game::handleMenuOpen);
    connect(m_menu, &GameMenu::menuClosed, this, &Game::handleMenuClosed);

    m_spawnTimer->setInterval(m_spawnDelay_ms);
    connect(m_spawnTimer, &QTimer::timeout, this, &Game::spawnPokemon);
    m_spawnTimer->start();

    handleMenuOpen();
}

Game::~Game() {
    m_spawnTimer->stop();
    disconnect(m_trayIcon, nullptr, this, nullptr);
    disconnect(m_menu, nullptr, this, nullptr);
    disconnect(m_spawnTimer, nullptr, this, nullptr);

    if (m_activeBattle) {
        disconnect(m_activeBattle, nullptr, this, nullptr);
        disconnect(this, nullptr, m_activeBattle, nullptr);
        delete m_activeBattle;
        m_activeBattle = nullptr;
    }

    if (m_wildPokemon) {
        disconnect(m_wildPokemon, nullptr, this, nullptr);
        disconnect(this, nullptr, m_wildPokemon, nullptr);
        delete m_wildPokemon;
        m_wildPokemon = nullptr;
    }

    delete m_menu;
}

void Game::safelyRemoveBattleScene(){
    if (!m_activeBattle) return;
    disconnect(m_activeBattle, nullptr, this, nullptr);
    disconnect(this, nullptr, m_activeBattle, nullptr);
    m_activeBattle->deleteLater();
    m_activeBattle = nullptr;
}

void Game::safelyRemoveWildPokemon(){
    if (!m_wildPokemon) return;
    disconnect(m_wildPokemon, nullptr, this, nullptr);
    disconnect(this, nullptr, m_wildPokemon, nullptr);
    m_wildPokemon->deleteLater();
    m_wildPokemon = nullptr;
};

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

    QTimer::singleShot(0, this, nullptr);
    if (active){
        spawnPokemon();
    }
    else{
        if (m_wildPokemon) {
            m_spawnPoint = m_wildPokemon->position();
            m_spawnDirection = m_wildPokemon->m_currentDirection;
            safelyRemoveWildPokemon();
        }
        if (m_activeBattle) {
            m_spawnPoint = m_activeBattle->position() + m_activeBattle->m_spriteOffset;
            m_spawnDirection = m_activeBattle->m_currentDirection;
            m_activeBattle->setSceneVisibility(false);
            safelyRemoveBattleScene();
        }
    }
    processing = false;
}

void Game::initializeGame() {
    m_party = m_db.getParty();
    bool hasParty = false;
    for (const auto& poke : m_party) {
        if (poke._id>0) {
            hasParty = true;
            break;
        }
    }

    if (!hasParty) {
        qDebug() << "New game detected, creating initial Pokemon...";
        createInitialPokemon();
        m_party = m_db.getParty();
    } else {
        qDebug() << "Loading saved game...";
    }

    PokemonState wildState = m_db.getWildPokemon();
    if (wildState.pokedex_id > 0) {
        qDebug() << "Found wild Pokemon in database:" << QString::fromStdString(Lookup::getPoke(wildState.pokedex_id)->name);
    }
}

void Game::loadParty() {
    m_party = m_db.getParty();
    m_partyDirty = false;
}

void Game::spawnPokemon() {
    if (m_wildPokemon) return;

    PokemonState wildState = m_db.getWildPokemon();
    if (wildState.pokedex_id > 0) {
        qDebug() << "Spawning existing wild Pokemon:" << QString::fromStdString(Lookup::getPoke(wildState.pokedex_id)->name);
    } else {
        m_spawnDirection = rand()%4;
        m_spawnPoint = QPoint(-1,-1);

        std::uniform_int_distribution<int> dist(1, 493);
        int pokedexId = dist(m_rng);
        const Poke* wildPoke = Lookup::getPoke(pokedexId);

        wildState = {};
        wildState.pokedex_id = pokedexId;
        wildState.name = wildPoke->name;
        wildState.lvl = 15;
        wildState.nature = Nature::Hardy;
        wildState.moves[0] = 1;
        wildState.moves[1] = 422;
        wildState.moves[2] = 86;
        wildState.moves[3] = 86;

        m_db.spawnWildPokemon(wildState);
        qDebug() << "Created new wild Pokemon:" << QString::fromStdString(wildState.name);
    }

    m_wildPokemon = new WildPokemon(wildState.pokedex_id, m_spawnPoint, m_spawnDirection);
    connect(m_wildPokemon, &WildPokemon::startABattle, this, &Game::handleBattleStart);
    m_wildPokemon->show();
    m_spawnTimer->stop();
}

void Game::handleBattleStart() {
    if (!m_wildPokemon || m_activeBattle) return;

    m_spawnPoint = m_wildPokemon->position();
    m_spawnDirection = m_wildPokemon->m_currentDirection;

    PokemonState wildState = m_db.getWildPokemon();
    std::vector<PokemonState> partyVec;
    for (const auto& poke : m_party) {
        if (poke._id>0) {
            partyVec.push_back(poke);
        }
    }

    auto battleMoveHandler = std::make_unique<BattleMoveHandler>(wildState, m_party, m_rng);

    Party battleParty;
    for(int slot = 0; slot < 6; ++slot) {
        const PokemonState& pokemon = m_party[slot];
        if(pokemon._id>0) continue;

        const AssetInfo* info = Lookup::getSpriteInfo(pokemon.pokedex_id);
        battleParty.pokedexIds[slot] = pokemon.pokedex_id;
        battleParty.spriteIds[slot] = info->rowId;
        battleParty.names[slot] = pokemon.name;
        battleParty.lvls[slot] = pokemon.lvl;
        battleParty.ballIds[slot] = pokemon.pokeball_id;
        battleParty.healthTotals[slot] = PokeMath::calculateHealth(pokemon.lvl, Lookup::getPoke(pokemon.pokedex_id)->base_stats[0], 0, 0);

        for (int moveSlot = 0; moveSlot<4; moveSlot++){
            int moveId = pokemon.moves[moveSlot];
            if(moveId<1) continue;
            const Move* _move = Lookup::getMove(moveId);
            battleParty.moves[slot][moveSlot] = {_move->name, PokeTypes::typeToString(_move->type)};
        }
    }

    m_activeBattle = new Battle(m_spawnPoint, m_spawnDirection, wildState, battleParty, std::move(battleMoveHandler));
    connect(m_activeBattle, &Battle::battleEnded, this, &Game::handleBattleEnd);
    connect(m_activeBattle, &Battle::_updatePartyXP, this, &Game::updatePartyXP);

    QTimer::singleShot(80, this, [this]() {
        safelyRemoveWildPokemon();
    });

    qDebug() << "Starting battle...";
}

void Game::handleBattleEnd(const char* endState, bool removeWild) {
    bool opponentCaught = (strcmp(endState, "OpponentCaught") == 0);

    if (removeWild) {
        if (opponentCaught) {
            int ballIndex = m_activeBattle->getQMLSceneProperty<int>("currentOpponentBallIndex");
            int caughtId = m_db.catchWildPokemon(ballIndex);
            if (caughtId > 0) {
                for (int i = 0; i < 6; i++) {
                    if (m_party[i]._id>0) {
                        PokemonState newPokemon = m_db.getPokemon(caughtId);
                        m_db.moveToParty(caughtId, i);
                        m_party[i] = newPokemon;
                        break;
                    }
                }
            }
        }

        m_db.clearWild();
        safelyRemoveBattleScene();
        m_spawnPoint = QPoint(-1,-1);
        m_spawnTimer->start();

    } else {
        m_activeBattle->handleDrag(false);
        m_spawnPoint = m_activeBattle->position() + m_activeBattle->m_spriteOffset;
        m_spawnDirection = m_activeBattle->m_currentDirection;
        spawnPokemon();

        QTimer::singleShot(100, this, [this]() {
            safelyRemoveBattleScene();
        });
    }
}

void Game::createInitialPokemon() {
    PokemonState gastly;
    gastly.pokedex_id = 336;
    gastly.name = "Gastly";
    gastly.pokeball_id = 0;
    gastly.nature = Nature::Hardy;
    gastly.lvl = 10;
    gastly.moves[0] = 202;
    gastly.moves[1] = 28;
    gastly.moves[2] = 339;
    gastly.moves[3] = 93;

    int gastlyId = m_db.createPokemon(gastly);
    if (gastlyId > 0) {
        qDebug() << "Created Gastly with database ID:" << gastlyId;
        m_db.moveToParty(gastlyId, 0);
    }

    PokemonState wailord;
    wailord.pokedex_id = 321;
    wailord.name = "Wailord";
    wailord.pokeball_id = 0;
    wailord.nature = Nature::Hardy;
    wailord.lvl = 10;
    wailord.moves[0] = 48;
    wailord.moves[1] = 28;
    wailord.moves[2] = 339;
    wailord.moves[3] = 260;

    int wailordId = m_db.createPokemon(wailord);
    if (wailordId > 0) {
        qDebug() << "Created Wailord with database ID:" << wailordId;
        m_db.moveToParty(wailordId, 1);
    }

    PokemonState groudon;
    groudon.pokedex_id = 383;
    groudon.name = "Oysterhead";
    groudon.pokeball_id = 2;
    groudon.nature = Nature::Hardy;
    groudon.lvl = 10;
    groudon.moves[0] = 14;
    groudon.moves[1] = 53;
    groudon.moves[2] = 426;
    groudon.moves[3] = 434;

    int groudonId = m_db.createPokemon(groudon);
    if (groudonId > 0) {
        qDebug() << "Created Groudon with database ID:" << groudonId;
        m_db.moveToParty(groudonId, 2);
    }
}

void Game::updatePartyXP(std::array<int, 6> spread) {
    if (!m_activeBattle) return;

    std::array<int,6> lvlUps = {-1,-1,-1,-1,-1,-1};
    std::vector<PokemonState> updates;

    for (int i = 0; i < 6; i++) {
        if (spread[i] > 0 && m_party[i]._id>0) {
            int xpGain = spread[i];
            int oldXP = m_party[i].currentXP;
            int oldLevel = m_party[i].lvl;

            m_party[i].currentXP += xpGain;

            while (m_party[i].lvl < 100) {
                int xpNeeded = PokeMath::xpToNextLevel(m_party[i].lvl);
                if (m_party[i].currentXP >= xpNeeded) {
                    m_party[i].currentXP -= xpNeeded;
                    m_party[i].lvl++;
                    lvlUps[i] = m_party[i].lvl;
                } else {
                    break;
                }
            }

            int xpToNext = PokeMath::xpToNextLevel(m_party[i].lvl);
            qDebug().nospace()
                << "[#" << m_party[i]._id << "] " << QString::fromStdString(m_party[i].name)
                << ": Lvl " << oldLevel << "→" << m_party[i].lvl
                << ", XP " << oldXP << "→" << m_party[i].currentXP
                << " (+" << xpGain << ")"
                << ", XP For Next lvl: " << xpToNext;

            updates.push_back(m_party[i]);
        }
    }

    if (!updates.empty()) {
        m_db.batchUpdatePokemon(updates);
    }

    m_activeBattle->showXPAndEndBattle(spread, lvlUps);
}

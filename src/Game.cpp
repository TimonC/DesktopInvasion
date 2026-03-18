#include <Game.h>
#include <PokeTypes.h>
#include <SystemTrayIcon.h>
#include <WildPokemon.h>
#include <globals.h>
#include <PokeMath.h>
#include <QTimer>
#include <QDebug>
#include <cstring>
#include <form_mapper.h>
#include "BattleMoveHandler.h"

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
    if (m_activeBattle) safelyRemoveBattleScene();
    if (m_wildPokemon) safelyRemoveWildPokemon();
    delete m_menu;
}

void Game::safelyRemoveBattleScene(){
    assert(m_activeBattle && "There should be no existing Battle when this is called.");

    disconnect(m_activeBattle, nullptr, this, nullptr);
    disconnect(this, nullptr, m_activeBattle, nullptr);

    m_activeBattle->deleteLater();
    m_activeBattle = nullptr;
}

void Game::safelyRemoveWildPokemon(){
    assert(m_wildPokemon && "There should be no existing WildPokemon instance when this is called.");

    disconnect(m_wildPokemon, nullptr, this, nullptr);
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

        // Singleshot to ensure we're in next event loop iteration
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
    assert(!(m_wildPokemon) && "Spawn pokemon called with existing instance, there should only ever be one.");

    PokemonState wildState = m_db.getWildPokemon();
    if (wildState.pokedex_id > 0) {
        m_wildPokemonInfo = Globals::getPokemonInfo(wildState.pokedex_id);
        qDebug() << "Spawning existing wild Pokemon:" << QString::fromStdString(wildState.name);
    } else {

        m_spawnDirection = rand()%4;
        m_spawnPoint = QPoint(-1,-1);

        m_wildPokemonInfo = Globals::getPokemonInfo();

        PokemonState newWild;
        newWild.pokedex_id = m_wildPokemonInfo->pokedexId;
        newWild.name = m_wildPokemonInfo->name;
        newWild.lvl = 40;

        for (int i = 0; i < 6; i++) {
            newWild.ivs[i] = 32;
            newWild.evs[i] = 0;
        }
        newWild.nature = Nature::Hardy;

        newWild.moves[0] = 1;
        newWild.moves[1] = 109;
        newWild.moves[2] = 181;
        newWild.moves[3] = 47;

        m_db.spawnWildPokemon(newWild);
        qDebug() << "Created new wild Pokemon:" << QString::fromStdString(newWild.name);
    }

    if (m_wildPokemonInfo) {
        m_wildPokemon = new WildPokemon(m_wildPokemonInfo, m_spawnPoint, m_spawnDirection);
        connect(m_wildPokemon, &WildPokemon::startABattle, this, &Game::handleBattleStart);

        m_wildPokemon->show();
        m_spawnTimer->stop();
    }
    /* m_wildPokemon->startABattle(); */
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
        party.lvls[i] = pokemon.lvl;
        party.gens[i] = info->generation;
        party.ballIds[i] = pokemon.pokeball_id;
        party.healthTotals[i] = PokeMath::calculateHealth(pokemon.lvl, Globals::getPoke(info->pokedexId)->base_stats[0], pokemon.ivs[0], pokemon.evs[0]);

        for (int moveSlot = 0; moveSlot<4; moveSlot++){
           int moveId = pokemon.moves[moveSlot];
           if(moveId<1) continue;

           const Move* _move = Globals::getMove(moveId);
           party.moves[i][moveSlot] = {_move->name, PokeTypes::typeToString(_move->type)};
        };

    }

    return party;
}
void Game::handleBattleStart() {
    if (!m_wildPokemon || m_activeBattle) {
        qWarning() << "Cannot start battle - invalid state";
        return;
    }

    m_spawnPoint = m_wildPokemon->position();
    m_spawnDirection = m_wildPokemon->m_currentDirection;

    //Cleanup with delay for smooth transition from WildPokemon to Battle
    QTimer::singleShot(200, this, [this]() {
        safelyRemoveWildPokemon();
    });

    // Gather all IDs (wild + party)
    std::vector<int> idsToFetch = {0}; // Wild Pokemon ID at index 0
    for(int i = 0; i < 6; i++){
        idsToFetch.push_back(m_partyIds[i]);
    }

    std::vector<PokemonState> pokemonStates = m_db.getPokemonBatch(idsToFetch);

    //Create BattleMoveHandler
    PokemonState wildState = pokemonStates[0];
    std::array<PokemonState, 6> partyStates;
    for(int i = 0; i < 6; i++){
        partyStates[i] = pokemonStates[i + 1];
    }
    auto battleMoveHandler = std::make_unique<BattleMoveHandler>(wildState, partyStates);

    const Party party = getParty();


    m_activeBattle = new Battle(m_spawnPoint, m_spawnDirection, wildState, party, std::move(battleMoveHandler));

    connect(m_activeBattle, &Battle::battleEnded,
            this, &Game::handleBattleEnd);
    connect(m_activeBattle, &Battle::_updatePartyXP,
            this, &Game::updatePartyXP);

    qDebug() << "Starting battle...";
}

void Game::handleBattleEnd(const char* endState, bool removeWild) {
    assert((   !std::strcmp(endState, "PlayerWon")
            || !std::strcmp(endState, "PlayerRun")
            || !std::strcmp(endState, "OpponentWon")
            || !std::strcmp(endState, "OpponentCaught"))
           && "Action must be 'Switch', 'Fight' or 'Catch'");
    qDebug() << endState;

    bool playerWon = (strcmp(endState, "PlayerWon") == 0);
    bool opponentCaught = (strcmp(endState, "OpponentCaught") == 0);

    if (removeWild) {
        if (opponentCaught) {
            int ballIndex = m_activeBattle->getQMLSceneProperty<int>("currentOpponentBallIndex");

            int caughtId = m_db.catchWildPokemon(ballIndex);
            if (caughtId > 0) {
                PokemonState caughtPokemon = m_db.getPokemon(caughtId);
                QString caughtName = QString::fromStdString(caughtPokemon.name);

                qDebug() << caughtName << "caught! Database ID:" << caughtId;

                // Add to first empty party slot, if possible
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

        // Cleanup wild pokemon and battle
        if( m_db.clearWild()){
            qDebug() << "Cleared wild pokemon instance";
        }else{
            qWarning() << "Failed to clear wild pokemon";
        }

        safelyRemoveBattleScene();
        m_spawnPoint = QPoint(-1,-1);
        m_spawnTimer->start();

    } else {
        // Battle ended without removing wild Pokemon
        m_activeBattle->handleDrag(false);
        m_spawnPoint = m_activeBattle->position() + m_activeBattle->m_spriteOffset;
        m_spawnDirection = m_activeBattle->m_currentDirection;

        spawnPokemon();

        //Cleanup with delay for smooth transition from battlescene to wild pokemon
        QTimer::singleShot(100, this, [this]() {
            safelyRemoveBattleScene();
        });
    }
}


void Game::createInitialPokemon() {
    PokemonState duskull;
    duskull.pokedex_id = 355;
    duskull.name = "Duskull";

    for (int i = 0; i < 6; i++) {
        duskull.ivs[i] = 32;
        duskull.evs[i] = 50;
    }

    duskull.nature = Nature::Hardy;
    duskull.lvl = 10;
    duskull.moves[0] = 261;
    duskull.moves[1] = 425;
    duskull.moves[2] = 181;
    duskull.moves[3] = 47;

    int pokemonId = m_db.createPokemon(duskull);
    if (pokemonId > 0) {
        qDebug() << "Created Duskull with database ID:" << pokemonId;
        m_db.setPartyPokemon(0, pokemonId);
        m_partyIds[0] = pokemonId;
    }

    PokemonState dusclops;
    dusclops.pokedex_id = 356;
    dusclops.name = "Dusclops";
    dusclops.pokeball_id = 1;

    for (int i = 0; i < 6; i++) {
        dusclops.ivs[i] = 32;
        dusclops.evs[i] = 50;
    }

    dusclops.nature = Nature::Hardy;
    dusclops.lvl = 100;
    dusclops.moves[0] = 45;
    dusclops.moves[1] = 86;
    dusclops.moves[2] = 109;//425;
    dusclops.moves[3] = 53;

    int pokemonId2 = m_db.createPokemon(dusclops);
    if (pokemonId2 > 0) {
        qDebug() << "Created Dusclops with database ID:" << pokemonId2;
        m_db.setPartyPokemon(1, pokemonId2);
        m_partyIds[1] = pokemonId;
    }

}
void Game::updatePartyXP(std::array<int, 6> spread) {
    if (!m_activeBattle) {
        qWarning() << "Cannot show XP sequence - battle already ended";
        return;
    }

    std::array<int,6> lvlUps = {-1,-1,-1,-1,-1,-1};
    std::vector<int> idsToUpdate;

    for (int i = 0; i < 6; i++) {
        if (spread[i] > 0 && m_partyIds[i] > 0) {
            idsToUpdate.push_back(m_partyIds[i]);
        }
    }

    if (idsToUpdate.empty()) {
        qDebug() << "No Pokémon received XP (all spread values were 0 or negative)";
        return;
    }

    // Single batch fetch
    std::vector<PokemonState> originalPokemon = m_db.getPokemonBatch(idsToUpdate);
    std::vector<PokemonState> updatedPokemon = originalPokemon;

    // Process all updates in memory
    for (size_t idx = 0; idx < updatedPokemon.size(); idx++) {
        PokemonState& pokemon = updatedPokemon[idx];
        PokemonState& original = originalPokemon[idx];

        // Find which party slot this is
        bool found = false;
        for (int i = 0; i < 6; i++) {
            if (m_partyIds[i] == pokemon._id && spread[i] > 0) {
                int xpGain = spread[i];
                int oldXP = pokemon.currentXP;
                int oldLevel = pokemon.lvl;

                // Add XP
                pokemon.currentXP += xpGain;

                // Check for level ups
                while (pokemon.lvl < 100) {
                    int xpNeeded = PokeMath::xpForNextLevel(pokemon.lvl, pokemon.currentXP);
                    if (xpNeeded > 0) break;
                    pokemon.lvl++;
                    lvlUps[i] = pokemon.lvl;
                }

                // Calculate XP needed for next level
                int xpForNext = PokeMath::xpForNextLevel(pokemon.lvl, pokemon.currentXP);

                // Debug output for this Pokémon
                qDebug().nospace()
                    << "[" << i << "] " << QString::fromStdString(pokemon.name)
                    << ": XP " << oldXP << "→" << pokemon.currentXP
                    << " (+" << xpGain << ")"
                    << ", Lvl " << oldLevel << "→" << pokemon.lvl
                    << ", NextLvlXP " << xpForNext;

                found = true;
                break;
            }
        }

        if (!found) {
            qWarning() << "Pokemon ID" << pokemon._id << "not found in party or no XP gain";
        }
    }

    if (m_db.batchUpdatePokemon(updatedPokemon)) {
        qDebug() << "Successfully batch updated" << updatedPokemon.size() << "Pokémon";
    } else {
        qWarning() << "Failed to batch update Pokémon";
    }

    m_activeBattle->showXPAndEndBattle(spread, lvlUps);
}

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
        qDebug() << "Found wild Pokemon in database:" << QString::fromStdString(Lookup::getPoke(wildState.pokedex_id)->name);
    }
}


void Game::loadParty() {
    GameState state = m_db.loadGameState();
    m_partyIds = {state.party_id[0], state.party_id[1], state.party_id[2],
                  state.party_id[3], state.party_id[4], state.party_id[5]};
    m_partyDirty = true;
}

void Game::spawnPokemon() {
    assert(!(m_wildPokemon) && "Spawn pokemon called with existing instance, there should only ever be one.");

    PokemonState wildState = m_db.getWildPokemon();
    if (wildState.pokedex_id > 0) {
        qDebug() << "Spawning existing wild Pokemon:" << QString::fromStdString(Lookup::getPoke(wildState.pokedex_id)->name);
    } else {
        m_spawnDirection = rand()%4;
        m_spawnPoint = QPoint(-1,-1);



        std::uniform_int_distribution<int> dist(1, 493);
        int pokedexId = dist(m_rng);
        /* int pokedexId = 493; */
        const Poke* wildPoke = Lookup::getPoke(pokedexId);

        wildState = {};
        wildState.pokedex_id = pokedexId;
        wildState.name = wildPoke->name;
        wildState.lvl = 15;

        for (int i = 0; i < 6; i++) {
            wildState.ivs[i] = 32;
            wildState.evs[i] = 0;
        }
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

Party Game::getParty() {
    if (m_partyDirty) {
        updatePartyCache();
        m_partyDirty = false;
    }
    return m_cachedParty;
}

void Game::updatePartyCache() {
    GameState state = m_db.loadGameState();
    std::array<PokemonState, 6> partyStates;

    std::vector<int> idsToFetch;
    for(int i = 0; i < 6; ++i) {
        if(state.party_id[i] > 0) {
            idsToFetch.push_back(state.party_id[i]);
        }
    }

    if (!idsToFetch.empty()) {
        std::vector<PokemonState> batch = m_db.getPokemonBatch(idsToFetch);
        for(size_t i = 0; i < idsToFetch.size(); ++i) {
            for(int slot = 0; slot < 6; ++slot) {
                if(state.party_id[slot] == idsToFetch[i]) {
                    partyStates[slot] = batch[i];
                    break;
                }
            }
        }
    }

    m_cachedParty = createPartyFromStates(partyStates);
}

void Game::fillPartySlot(Party& party, int slot, const PokemonState& pokemon) {
    const AssetInfo* info = Lookup::getSpriteInfo(pokemon.pokedex_id);
    party.pokedexIds[slot] = pokemon.pokedex_id;
    party.spriteIds[slot] = info->rowId;
    party.names[slot] = pokemon.name;
    party.lvls[slot] = pokemon.lvl;
    party.ballIds[slot] = pokemon.pokeball_id;
    party.healthTotals[slot] = PokeMath::calculateHealth(pokemon.lvl, Lookup::getPoke(pokemon.pokedex_id)->base_stats[0], pokemon.ivs[0], pokemon.evs[0]);

    for (int moveSlot = 0; moveSlot<4; moveSlot++){
        int moveId = pokemon.moves[moveSlot];
        if(moveId<1) continue;
        const Move* _move = Lookup::getMove(moveId);
        party.moves[slot][moveSlot] = {_move->name, PokeTypes::typeToString(_move->type)};
    }
}

Party Game::createPartyFromStates(const std::array<PokemonState, 6>& partyStates) {
    Party party;
    for(int slot = 0; slot < 6; ++slot) {
        const PokemonState& pokemon = partyStates[slot];
        if(pokemon._id <= 0) continue;
        fillPartySlot(party, slot, pokemon);
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

    QTimer::singleShot(200, this, [this]() {
        safelyRemoveWildPokemon();
    });

    std::vector<int> idsToFetch = {0};
    for(int i = 0; i < 6; i++){
        idsToFetch.push_back(m_partyIds[i]);
    }

    auto startTime = std::chrono::steady_clock::now();
    std::vector<PokemonState> pokemonStates = m_db.getPokemonBatch(idsToFetch);
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    qDebug() << "Database batch fetch took:" << duration.count() << "ms";

    PokemonState wildState = pokemonStates[0];
    std::array<PokemonState, 6> partyStates;
    for(int i = 0; i < 6; i++){
        partyStates[i] = pokemonStates[i + 1];
    }
    auto battleMoveHandler = std::make_unique<BattleMoveHandler>(wildState, partyStates, m_rng);

    Party party = createPartyFromStates(partyStates);

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
                for (int i = 0; i < 6; i++) {
                    if (m_partyIds[i] == 0) {
                        m_db.setPartyPokemon(i, caughtId);
                        m_partyIds[i] = caughtId;
                        m_partyDirty = true;
                        break;
                    }
                }
            }
        }

        if( m_db.clearWild()){
            qDebug() << "Cleared wild pokemon instance";
        }else{
            qWarning() << "Failed to clear wild pokemon";
        }

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
    gastly.pokedex_id = 92;
    gastly.name = "Gastly";
    gastly.pokeball_id = 0;

    for (int i = 0; i < 6; i++) {
        gastly.ivs[i] = 32;
        gastly.evs[i] = 50;
    }

    gastly.nature = Nature::Hardy;
    gastly.lvl = 10;
    gastly.moves[0] = 202;
    gastly.moves[1] = 28;
    gastly.moves[2] = 339;
    gastly.moves[3] = 260;


    int pokemonId1 = m_db.createPokemon(gastly);
    if (pokemonId1 > 0) {
        qDebug() << "Created gastly with database ID:" << pokemonId1;
        m_db.setPartyPokemon(0, pokemonId1);
        m_partyIds[0] = pokemonId1;
    }

    PokemonState wailord;
    wailord.pokedex_id = 336;
    wailord.name = "Wailord";
    wailord.pokeball_id = 0;

    for (int i = 0; i < 6; i++) {
        wailord.ivs[i] = 32;
        wailord.evs[i] = 50;
    }

    wailord.nature = Nature::Hardy;
    wailord.lvl = 10;
    wailord.moves[0] = 202;
    wailord.moves[1] = 28;
    wailord.moves[2] = 339;
    wailord.moves[3] = 260;


    int pokemonId2 = m_db.createPokemon(wailord);
    if (pokemonId2 > 0) {
        qDebug() << "Created wailord with database ID:" << pokemonId2;
        m_db.setPartyPokemon(1, pokemonId2);
        m_partyIds[1] = pokemonId2;
    }

    PokemonState groudon;

    groudon.pokedex_id = 383;
    groudon.name = "Oysterhead";
    groudon.pokeball_id = 2;

    for (int i = 0; i < 6; i++) {
        groudon.ivs[i] = 32;
        groudon.evs[i] = 50;
    }

    groudon.nature = Nature::Hardy;
    groudon.lvl = 10;
    groudon.moves[0] = 14;
    groudon.moves[1] = 104;
    groudon.moves[2] = 426;
    groudon.moves[3] = 434;

    int pokemonId = m_db.createPokemon(groudon);
    if (pokemonId > 0) {
        qDebug() << "Created groudon with database ID:" << pokemonId;
        m_db.setPartyPokemon(2, pokemonId);
        m_partyIds[2] = pokemonId;
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
        m_activeBattle->showXPAndEndBattle(spread, lvlUps);
        return;
    }

    std::vector<PokemonState> originalPokemon = m_db.getPokemonBatch(idsToUpdate);
    std::vector<PokemonState> updatedPokemon = originalPokemon;

    for (size_t idx = 0; idx < updatedPokemon.size(); idx++) {
        PokemonState& pokemon = updatedPokemon[idx];

        bool found = false;
        for (int i = 0; i < 6; i++) {
            if (m_partyIds[i] == pokemon._id && spread[i] > 0) {
                int xpGain = spread[i];
                int oldXP = pokemon.currentXP;
                int oldLevel = pokemon.lvl;

                pokemon.currentXP += xpGain;

                while (pokemon.lvl < 100) {
                    int xpNeeded = PokeMath::xpToNextLevel(pokemon.lvl);

                    if (pokemon.currentXP >= xpNeeded) {
                        pokemon.currentXP -= xpNeeded;
                        pokemon.lvl++;
                        lvlUps[i] = pokemon.lvl;
                    } else {
                        break;
                    }
                }

                int xpToNext = PokeMath::xpToNextLevel(pokemon.lvl);
                qDebug().nospace()
                    << "[#" << pokemon._id << "] " << QString::fromStdString(pokemon.name)
                    << ": Lvl " << oldLevel << "→" << pokemon.lvl
                    << ", XP " << oldXP << "→" << pokemon.currentXP
                    << " (+" << xpGain << ")"
                    << ", XP For Next lvl: " << xpToNext;

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
        m_partyDirty = true;
    } else {
        qWarning() << "Failed to batch update Pokémon";
    }

    m_activeBattle->showXPAndEndBattle(spread, lvlUps);
}

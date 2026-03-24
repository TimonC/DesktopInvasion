#include <Game.h>
#include <PokeTypes.h>
#include <SystemTrayIcon.h>
#include <WildPokemon.h>
#include <globals.h>
#include <PokeMath.h>
#include <QTimer>
#include <QDebug>
#include <QVariantMap>
#include <cstring>
#include <BattleMoveHandler.h>
#include <data_poke_asset.h>
#include <lookup.h>
#include <cassert>
#include <algorithm>

Game::Game(QWindow* parent)
    : QObject(parent)
    , m_menu()
    , m_trayIcon(new SystemTrayIcon(this))
    , m_spawnTimer(new QTimer(this))
    , m_rng(std::random_device{}())
{
    qDebug() << "Game constructor called!";

    qApp->setQuitOnLastWindowClosed(false);
    initializeGame();
}

Game::~Game() {
    qDebug() << "Game destructor called!";
    resetGame();

}

void Game::resetGame(){
    m_spawnTimer->stop();

    if (m_menu) {
        disconnect(m_menu, nullptr, this, nullptr);
        disconnect(this, nullptr, m_menu, nullptr);
        delete m_menu;
        m_menu = nullptr;
    }

    if (m_starterMenu){
        disconnect(m_starterMenu, nullptr, this, nullptr);
        disconnect(this, nullptr, m_starterMenu, nullptr);
        delete m_starterMenu;
        m_starterMenu = nullptr;
    }

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
}

void Game::safelyRemoveBattleScene() {
    if (!m_activeBattle) return;
    disconnect(m_activeBattle, nullptr, this, nullptr);
    disconnect(this, nullptr, m_activeBattle, nullptr);
    m_activeBattle->deleteLater();
    m_activeBattle = nullptr;
}

void Game::safelyRemoveWildPokemon() {
    if (!m_wildPokemon) return;
    disconnect(m_wildPokemon, nullptr, this, nullptr);
    disconnect(this, nullptr, m_wildPokemon, nullptr);
    m_wildPokemon->deleteLater();
    m_wildPokemon = nullptr;
}

void Game::setGameActive(bool active) {
    static bool processing = false;
    if (processing) return;
    m_gameUsedToBeActive = active;
    processing = true;

    QTimer::singleShot(0, this, nullptr);
    if (active) {
        spawnPokemon();
    } else {
        if (m_wildPokemon) {
            m_spawnPoint     = m_wildPokemon->position();
            m_spawnDirection = m_wildPokemon->m_currentDirection;
            safelyRemoveWildPokemon();
        }
        if (m_activeBattle) {
            m_spawnPoint     = m_activeBattle->position() + m_activeBattle->m_spriteOffset;
            m_spawnDirection = m_activeBattle->m_currentDirection;
            m_activeBattle->setSceneVisibility(false);
            safelyRemoveBattleScene();
        }
    }
    processing = false;
}

void Game::initializeGame(bool openStarter) {
    bool hasParty = false;
    for (const auto& p : m_db.party())
        if (!p.empty()) { hasParty = true; break; }

    if (!hasParty && openStarter) {
        qDebug() << "Starting new game!";
        openStarterMenu();
        return;
    }
    else{
        qDebug() << "Save loaded — party size:" << m_db.partySize();
    }

    if (!m_db.wild().empty())
        qDebug() << "Resuming wild Pokemon:" << QString::fromStdString(m_db.wild().name);

    auto trainerNames = m_db.listTrainerNames();
    m_trayIcon->createContextMenu(trainerNames, m_db.currentSaveId());

    connect(m_trayIcon, &SystemTrayIcon::gameActive,        this, &Game::setGameActive,    Qt::UniqueConnection);
    connect(m_trayIcon, &SystemTrayIcon::menuButtonPressed, this, &Game::handleMenuOpen,   Qt::UniqueConnection);
    connect(m_trayIcon, &SystemTrayIcon::saveSelected, this, &Game::handleSaveSelected,   Qt::UniqueConnection);
    connect(m_trayIcon, &SystemTrayIcon::newGameRequested, this, &Game::openStarterMenu,   Qt::UniqueConnection);
    connect(m_trayIcon, &SystemTrayIcon::deleteSaveRequested, this, &Game::deleteCurrentSave,   Qt::UniqueConnection);
    m_spawnTimer->setInterval(m_spawnDelay_ms);
    connect(m_spawnTimer, &QTimer::timeout, this, &Game::spawnPokemon,                     Qt::UniqueConnection);


    initMenu();
    Defaults d = m_db.loadDefaults();
    Globals::scale(d.scale);
    Globals::animationSpeed(d.speed);
    Globals::encounterLvlHigh(d.lvlRangeUp);
    Globals::encounterLvlLow(d.lvlRangeDown);
    Globals::expShare(d.expShareOn);
    m_menu->setDefaults(d);

    if(m_gameUsedToBeActive) m_spawnTimer->start();
}

void Game::handleSaveSelected(int saveId){
    if(m_db.currentSaveId()==saveId) return;
    m_db.setCurrentSaveId(saveId);
    resetGame();
    initializeGame();
}

void Game::deleteCurrentSave(){
    m_db.deleteSave(m_db.currentSaveId());
    auto v = m_db.listSaveIds();
    if(v.empty()){
        m_db.setCurrentSaveId(0);
        resetGame();
        openStarterMenu();
    }else{
        m_db.setCurrentSaveId(v.back());

        m_trayIcon->m_gameActive=false;
        m_trayIcon->toggleGameActive();
        m_trayIcon->show();
        m_gameUsedToBeActive = true;

        resetGame();
        initializeGame();
        setGameActive(true);
    }
}

void Game::openStarterMenu(){
    setGameActive(false);
    m_spawnTimer->stop();
    m_trayIcon->setVisible(false);

    m_starterMenu = new QQuickView();
    const char* env = getenv("DOCKER_ENV");
    if (env && strcmp(env, "dev") == 0)
        m_starterMenu->setSource(QUrl("../qml/qmlGameMenu/StarterMenu.qml"));
    else
        m_starterMenu->setSource(QUrl("qrc:/qml/qmlGameMenu/StarterMenu.qml"));

    m_starterMenu->setCursor(QCursor(QPixmap(":/assets/XY/pointer.png"), 6, 6));
    m_starterMenu->setTitle("DesktopInvasion");
    m_starterMenu->setResizeMode(QQuickView::SizeRootObjectToView);
    m_starterMenu->resize(1100, 720);
    m_starterMenu->show();

    QObject* root = m_starterMenu->rootObject();
    if (root)
        connect(root, SIGNAL(startGame(QString, int, int)), this, SLOT(onStarterMenuFinished(QString, int, int)));

    connect(m_starterMenu, &QQuickView::closing, this, [this](QQuickCloseEvent*){
        auto saves = m_db.listSaveIds();
        if (!saves.empty()) {
            if(m_trayIcon){
                m_trayIcon->m_gameActive=false;
                m_trayIcon->toggleGameActive();
                m_trayIcon->show();
            }
            m_gameUsedToBeActive = true;

            resetGame();
            initializeGame(false);
            setGameActive(true);
        } else {
            qApp->quit();
        }
    });
}

void Game::onStarterMenuFinished(QString playerName, int trainerId, int starterPokedexId){
    qDebug() << playerName << trainerId << starterPokedexId;

    PokemonState p;
    const Poke* poke = Lookup::getPoke(starterPokedexId);
    p.pokedex_id  = starterPokedexId;
    p.name        = poke->name;
    p.pokeball_id = 0;
    p.nature      = Nature::Hardy;
    p.lvl         = 10;
    p.moves[0]    = 1;

    int moveIndex = 0;
    for (int i = poke->eligible_move_count; i >= 0 && moveIndex < 4; i--) {
        int moveLevel = poke->eligible_moves[i].level;
        if (moveLevel > 0 && moveLevel <= p.lvl) {
            p.moves[moveIndex] = poke->eligible_moves[i].move_id;
            moveIndex++;
        }
    }


    GameState gs;
    gs.name             = playerName.toStdString();
    gs.player_sprite_id = trainerId;
    m_db.createNewSave(gs, p);

    disconnect(m_starterMenu, nullptr, this, nullptr);
    m_starterMenu->deleteLater();
    m_starterMenu = nullptr;

    m_gameUsedToBeActive = true;
    initializeGame();
    m_trayIcon->m_gameActive=false;
    m_trayIcon->toggleGameActive();
    m_trayIcon->show();
}


void Game::spawnPokemon() {
    if (m_wildPokemon) return;

    if (m_db.wild().empty()) {
        m_spawnDirection = rand() % 4;
        m_spawnPoint     = QPoint(-1, -1);

        int firstLvl = m_db.party().begin()->lvl;
        std::uniform_int_distribution<int> distLvl(firstLvl-Globals::encounterLvlLow(), firstLvl+Globals::encounterLvlHigh());
        int lvl = distLvl(m_rng);

        int pokedexId        = Lookup::getRandomPokemonByCatchRate(m_rng);
        const Poke* wildPoke = Lookup::getPoke(pokedexId);

        PokemonState w;
        w.pokedex_id = pokedexId;
        w.name       = wildPoke->name;
        w.lvl        = lvl;
        w.nature     = Nature::Hardy;

        const Poke* poke = Lookup::getPoke(pokedexId);
        int moveIndex = 0;
        for (int i = poke->eligible_move_count; i > 0 && moveIndex < 4; i--) {
            int moveLevel = poke->eligible_moves[i].level;
            if (moveLevel > 0 && moveLevel <= w.lvl) {
                w.moves[moveIndex] = poke->eligible_moves[i].move_id;
                moveIndex++;
            }
        }

        m_db.setWild(w);

    } else {
        qDebug() << "Resuming existing wild:" << QString::fromStdString(m_db.wild().name);
    }

    m_wildPokemon = new WildPokemon(m_db.wild().pokedex_id, m_spawnPoint, m_spawnDirection);
    connect(m_wildPokemon, &WildPokemon::startABattle, this, &Game::handleBattleStart);
    m_wildPokemon->show();
    m_spawnTimer->stop();
}
void Game::initMenu() {
    m_menu = new GameMenu();
    auto gs = m_db.loadGameState();

    std::string upperName = gs.name;
    std::transform(upperName.begin(), upperName.end(), upperName.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    m_menu->setTrainer(QString::fromStdString(upperName), gs.player_sprite_id);
    connect(m_menu, &GameMenu::menuClosed,          this, &Game::handleMenuClosed);
    connect(m_menu, &GameMenu::preloadBoxRequested,  this, &Game::handleMenuPreloadBox);
    connect(m_menu, &GameMenu::swapRequested,        this, &Game::handlePCSwap);
    connect(m_menu, &GameMenu::nameChangeRequested,  this, &Game::handleNameChange);
    connect(m_menu, &GameMenu::moveChangeRequested,  this, &Game::handleMoveChange);
}

QVariantMap Game::pokemonToMenuState(int slot, const PokemonState& p) {
    QVariantMap entry;
    entry["slot"]   = slot;
    entry["iconId"] = p.pokedex_id - 1;
    entry["name"]   = QString::fromStdString(p.name);
    entry["level"]  = p.lvl;
    entry["nature"] = QString::fromStdString(PokeTypes::natureToString(p.nature));

    const AssetInfo* info = Lookup::getSpriteInfo(p.pokedex_id);
    entry["rowId"] = info->rowId;
    entry["isBig"] = info->spriteSheet == SpriteSheet::Big;

    const Poke* poke = Lookup::getPoke(p.pokedex_id);
    auto stats = PokeMath::calculatePokeStats(p.lvl, poke->base_stats, PokeTypes::getNatureMultipliers(p.nature));
    QVariantList statsList;
    for (int value : stats)
        statsList.append(value);

    entry["pokeName"]   = QString::fromStdString(poke->name);
    entry["type1"]      = QString::fromStdString(PokeTypes::typeToString(poke->types[0]));
    entry["type2"]      = QString::fromStdString(PokeTypes::typeToString(poke->types[1]));
    entry["flavorText"] = Lookup::getRandomFlavorText(p.pokedex_id, m_rng);
    entry["stats"]      = statsList;

    std::vector<EligibleEntry> eligible_move;
    std::vector<EligibleEntry> eligible_tm_move;
    for (int i = 0; i < poke->eligible_move_count; i++) {
        if (p.lvl >= poke->eligible_moves[i].level) {
            if (poke->eligible_moves[i].level==-1) {
                eligible_tm_move.push_back({ poke->eligible_moves[i].level, poke->eligible_moves[i].move_id });
            } else {
                eligible_move.push_back({ poke->eligible_moves[i].level, poke->eligible_moves[i].move_id });
            }
        }
    }
    eligible_tm_move = m_db.filterKnownTMs(eligible_tm_move);
    std::vector<EligibleEntry> eligible;
    eligible.reserve(eligible_move.size() + eligible_tm_move.size());
    eligible.insert(eligible.end(), eligible_move.begin(), eligible_move.end());
    eligible.insert(eligible.end(), eligible_tm_move.begin(), eligible_tm_move.end());


    QVariantList eligibleMoves;
    for (const auto& e : eligible) {
        const Move* mv = Lookup::getMove(e.move_id);
        std::string name = mv->name;
        if (name.length() > 9)
            name.erase(std::remove(name.begin(), name.end(), '-'), name.end());
        QVariantMap moveData;
        moveData["id"]       = e.move_id;
        moveData["name"]     = QString::fromStdString(name);
        moveData["type"]     = QString::fromStdString(PokeTypes::typeToString(mv->type));
        moveData["power"]    = mv->power;
        moveData["accuracy"] = mv->accuracy;
        eligibleMoves.append(moveData);
    }
    entry["eligibleMoves"] = eligibleMoves;

    QVariantList moves;
    for (int moveSlot = 0; moveSlot < 4; moveSlot++) {
        if(p.moves[moveSlot]==0) break;

        const Move* mv = Lookup::getMove(p.moves[moveSlot]);
        std::string name = mv->name;
        if (name.length() > 9)
            name.erase(std::remove(name.begin(), name.end(), '-'), name.end());
        QVariantMap moveData;
        moveData["name"]     = QString::fromStdString(name);
        moveData["type"]     = QString::fromStdString(PokeTypes::typeToString(mv->type));
        moveData["flavor"]   = QString::fromStdString(mv->flavor_text);
        moveData["power"]    = mv->power;
        moveData["accuracy"] = mv->accuracy;
        moves.append(moveData);
    }
    entry["moves"] = moves;

    return entry;
}

QVariantList Game::partyToVariantList() {
    QVariantList list;
    const auto& party = m_db.party();
    for (int slot = 0; slot < PARTY_SIZE; ++slot) {
        const PokemonState& p = party[slot];
        if (p.empty()) continue;
        list.append(pokemonToMenuState(slot, p));
    }
    return list;
}

QVariantList Game::boxToVariantList(int boxIndex) {
    QVariantList list;
    m_db.loadBox(boxIndex);
    const auto& box = m_db.getBox(boxIndex);
    for (int slot = 0; slot < BOX_SIZE; ++slot) {
        const PokemonState& p = box[slot];
        if (p.empty()) continue;
        list.append(pokemonToMenuState(slot, p));
    }
    return list;
}

void Game::pushBoxToMenu(int boxIndex) {
    m_menu->loadBox(boxIndex, boxToVariantList(boxIndex));
    qDebug() << "[Game] Pushed PC box" << boxIndex << "to menu";
}

void Game::handleMenuPreloadBox(int boxIndex) {
    pushBoxToMenu(boxIndex);
}

void Game::handlePCSwap(int placex, int posx, int placey, int posy) {
    m_db.swapByPos(placex, posx, placey, posy);
}

void Game::handleNameChange(int placex, int posx, QString name) {
    m_db.renamePokemon(placex, posx, name.toStdString());
    if (placex == -1) {
        m_menu->loadParty(partyToVariantList(), false);
    } else {
        pushBoxToMenu(placex);
        m_menu->loadParty(partyToVariantList(), false);
    }
}

void Game::handleMoveChange(int placex, int posx, int moveSlot, int moveId) {
    m_db.setPokemonMove(placex, posx, moveSlot, moveId);
    if (placex == -1) {
        m_menu->loadParty(partyToVariantList(), false);
    } else {
        pushBoxToMenu(placex);
        m_menu->showBox(placex);
    }
}

void Game::handleMenuOpen() {
    bool usedToBeActive = m_gameUsedToBeActive;
    setGameActive(false);
    m_gameUsedToBeActive = usedToBeActive;

    Defaults d = m_db.loadDefaults();
    m_menu->setDefaults(d);

    m_menu->activate();

    m_menu->loadParty(partyToVariantList(), true);
    pushBoxToMenu(0);
    pushBoxToMenu(1);
    pushBoxToMenu(98);
    m_menu->showBox(0);

    m_trayIcon->enabled(false);
}

void Game::handleMenuClosed() {
    writeDefaults();
    m_trayIcon->enabled(true);
    if (m_gameUsedToBeActive) setGameActive(true);
}

void Game::writeDefaults() {
    Defaults d;
    d.scale        = Globals::scale();
    d.speed        = Globals::animationSpeed();
    d.lvlRangeUp   = Globals::encounterLvlHigh();
    d.lvlRangeDown = Globals::encounterLvlLow();
    d.expShareOn   = Globals::expShare();
    m_db.writeDefaults(d);
}

void Game::handleBattleStart() {
    if (!m_wildPokemon || m_activeBattle) {
        qDebug() << "handleBattleStart early return:"
                 << "wild=" << (m_wildPokemon != nullptr)
                 << "battle=" << (m_activeBattle != nullptr);
        return;
    }

    m_spawnPoint     = m_wildPokemon->position();
    m_spawnDirection = m_wildPokemon->m_currentDirection;

    const PokemonState& wildState = m_db.wild();
    const auto&         party     = m_db.party();

    auto battleMoveHandler = std::make_unique<BattleMoveHandler>(wildState, party, m_rng);

    Party battleParty;
    for (int slot = 0; slot < PARTY_SIZE; ++slot) {
        const PokemonState& pokemon = party[slot];
        if (pokemon.empty()) continue;

        const AssetInfo* info          = Lookup::getSpriteInfo(pokemon.pokedex_id);
        battleParty.pokedexIds[slot]   = pokemon.pokedex_id;
        battleParty.spriteIds[slot]    = info->rowId;
        battleParty.names[slot]        = pokemon.name;
        battleParty.lvls[slot]         = pokemon.lvl;
        battleParty.ballIds[slot]      = pokemon.pokeball_id;
        battleParty.healthTotals[slot] = PokeMath::calculateHealth(
            pokemon.lvl, Lookup::getPoke(pokemon.pokedex_id)->base_stats[0]);

        for (int m = 0; m < 4; ++m) {
            int moveId = pokemon.moves[m];
            if (moveId < 1) continue;
            const Move* mv = Lookup::getMove(moveId);
            battleParty.moves[slot][m] = {mv->name, PokeTypes::typeToString(mv->type)};
        }
    }

    std::array<int, 3> ballCount = m_db.loadPokeballs();
    m_activeBattle = new Battle(m_spawnPoint, m_spawnDirection, wildState, battleParty, ballCount, std::move(battleMoveHandler));

    connect(m_activeBattle, &Battle::battleEnded,    this, &Game::handleBattleEnd);
    connect(m_activeBattle, &Battle::_updatePartyXP, this, &Game::updatePartyXP);
    connect(m_activeBattle, &Battle::updateBallCount, this, [this](int delta, int row){
            m_db.changePokeball(delta, row);
        });

    QTimer::singleShot(80, this, [this]() { safelyRemoveWildPokemon(); });
}

void Game::handleBattleEnd(const char* endState, bool removeWild) {
    bool opponentCaught = (strcmp(endState, "OpponentCaught") == 0);

    if (removeWild) {
        if (opponentCaught) {
            int ballIndex    = m_activeBattle->getQMLSceneProperty<int>("currentOpponentBallIndex");
            auto [box, slot] = m_db.catchWildPokemon(ballIndex);

            if (box == -1 && slot >= 0)
                qDebug() << "Caught pokemon added to party slot" << slot;
            else if (box >= 0)
                qDebug() << "Caught pokemon sent to PC box" << box << "slot" << slot;
            else
                qDebug() << "Storage full — caught pokemon lost!";
        } else {
            m_db.clearWild();
        }

        safelyRemoveBattleScene();
        m_spawnPoint = QPoint(-1, -1);
        m_spawnTimer->start();

    } else {
        m_activeBattle->handleDrag(false);
        m_spawnPoint     = m_activeBattle->position() + m_activeBattle->m_spriteOffset;
        m_spawnDirection = m_activeBattle->m_currentDirection;
        spawnPokemon();

        QTimer::singleShot(100, this, [this]() { safelyRemoveBattleScene(); });
    }
}

void Game::updatePartyXP(std::array<int, 6> spread) {
    if (!m_activeBattle) return;

    std::array<int, 6> lvlUps = {-1, -1, -1, -1, -1, -1};

    for (int i = 0; i < PARTY_SIZE; ++i) {
        if (spread[i] <= 0) continue;

        PokemonState p = m_db.party()[i];
        if (p.empty()) continue;

        int xpGain = spread[i];
        int oldXP  = p.currentXP;
        int oldLvl = p.lvl;

        p.currentXP += xpGain;

        while (p.lvl < 100) {
            int xpNeeded = PokeMath::xpToNextLevel(p.lvl);
            if (p.currentXP >= xpNeeded) {
                p.currentXP -= xpNeeded;
                p.lvl++;
                lvlUps[i] = p.lvl;
            } else {
                break;
            }
        }

        qDebug().nospace()
            << QString::fromStdString(p.name)
            << " Lvl " << oldLvl << "->" << p.lvl
            << "  XP "  << oldXP  << "->" << p.currentXP
            << " (+" << xpGain << ")"
            << "  next: " << PokeMath::xpToNextLevel(p.lvl);

        m_db.setPartySlot(i, p);
    }

    int tmGet = 70;
    int ballGet = 2;
    std::uniform_int_distribution<int> dist(1, 3);
    int whichBall = dist(m_rng);

    m_db.changePokeball(ballGet, whichBall);
    m_db.addTechnicalMove(tmGet);

    m_activeBattle->showUpdateAndEndBattle(spread, lvlUps, tmGet, ballGet, whichBall);
}


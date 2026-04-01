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
#include <QMessageBox>
#include <QRandomGenerator>

Game::Game(QWindow* parent)
    : QObject(parent)
    , m_menu()
    , m_trayIcon(new SystemTrayIcon(this))
    , m_spawnTimer(new QTimer(this))
    , m_rng(initializeRNG())
{
    qApp->setQuitOnLastWindowClosed(false);
    initializeGame();
}

Game::~Game() {
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
        m_starterMenu->setSource(QUrl());
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

std::mt19937 Game::initializeRNG() {
    //Use system RNG for true random gameplay
    QRandomGenerator* secureGen = QRandomGenerator::system();

    std::vector<unsigned int> seeds;
    for (int i = 0; i < 8; ++i) {
        seeds.push_back(secureGen->generate());
    }

    std::seed_seq seq(seeds.begin(), seeds.end());
    return std::mt19937(seq);
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

void Game::setPetMode(bool active){
    if (active == m_petMode) return;
    m_petMode = active;

    Defaults d = m_db.loadDefaults();
    d.petModeOn = active;
    m_db.writeDefaults(d);

    setRandomSpawnPoint();
    if(m_wildPokemon){
        safelyRemoveWildPokemon();
        spawnPokemon();
    } else if(m_activeBattle){
        safelyRemoveBattleScene();
        spawnPokemon();
    }
}

void Game::setGameActive(bool active) {
    static bool processing = false;
    if (processing) return;
    m_gameUsedToBeActive = active;
    processing = true;

    if (active) {
        spawnPokemon();
        if(m_menuClosing) m_menuClosing=false;
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
    connect(m_trayIcon, &SystemTrayIcon::petActive,        this, &Game::setPetMode,    Qt::UniqueConnection);
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

    m_petMode = d.petModeOn;
    m_trayIcon->m_petActive = !d.petModeOn;
    m_trayIcon->togglePetMode();

    if(m_gameUsedToBeActive) m_spawnTimer->start();
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
    connect(m_menu, &GameMenu::evolvesRequested, this, &Game::handleEvolveRequest);
    connect(m_menu, &GameMenu::evolvePokemonRequested, this, &Game::handleEvolvePokemon);
}

void Game::handleMenuOpen() {
    bool usedToBeActive = m_gameUsedToBeActive;
    m_spawnTimer->stop();
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
    if (m_gameUsedToBeActive){
        m_menuClosing = true;
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

    const double menuWidth =  1200;
    const double menuHeight = 800;
    QRect availableGeometry = QGuiApplication::primaryScreen()->availableGeometry();

    const double scaleW = std::min(1.0, static_cast<double>(availableGeometry.width()) / menuWidth);
    const double scaleH = std::min(1.0, static_cast<double>(availableGeometry.height()) / menuHeight);
    const double uiScale = std::min(scaleW, scaleH);

    m_starterMenu->setProperty("uiScale", uiScale);
    m_starterMenu->resize(menuWidth, menuHeight);

    int x = (Globals::screenGeometry().width() - m_starterMenu->width()) / 2;
    int y = (Globals::screenGeometry().height() - m_starterMenu->height()) / 2;
    m_starterMenu->setPosition(x, y);
    m_starterMenu->show();

    QObject* root = m_starterMenu->rootObject();
    if (root)
        connect(root, SIGNAL(startGame(QString, QString, int, int)), this, SLOT(onStarterMenuFinished(QString, QString, int, int)));

    connect(m_starterMenu, &QQuickView::closing, this, [this](QQuickCloseEvent*){
        m_starterMenu->setSource(QUrl());
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

void Game::onStarterMenuFinished(const QString& playerName, const QString& nickName, int trainerId, int starterPokedexId){
    qDebug() << playerName << trainerId << starterPokedexId;

    PokemonState p;
    const Poke* poke = Lookup::getPoke(starterPokedexId);
    p.pokedex_id  = starterPokedexId;
    p.name        = nickName.toStdString();
    p.pokeball_id = 0;
    p.nature      = Lookup::getRandomNature(m_rng);
    p.lvl         = 10;

    int moveIndex = 0;
    for (int i = poke->eligible_move_count - 1; i >= 0 && moveIndex < 4; i--) {
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

    if (m_spawnPoint == QPoint(-1, -1)) {
        setRandomSpawnPoint();
    }

    if(!m_petMode){
        if (m_db.wild().empty()) {
            m_spawnDirection = rand() % 4;

            int firstLvl = m_db.party().begin()->lvl;
            std::uniform_int_distribution<int> distLvl(firstLvl-Globals::encounterLvlLow(), firstLvl+Globals::encounterLvlHigh());
            int lvl = std::clamp(distLvl(m_rng), 1, 100);

            std::vector<int> unavailableTmList = m_db.getTechnicalMoveList();
            const PokeRoll roll = Lookup::weightedSamplePokemon(lvl, unavailableTmList, m_rng);
            const Poke* wildPoke = Lookup::getPoke(roll.poke_id);

            m_tmGetId=roll.tmId;
            m_ballGetCount=roll.ballCount;
            m_ballGetId=roll.ballId;

            PokemonState w;
            w.pokedex_id = roll.poke_id;
            w.name       = wildPoke->name;
            w.lvl        = lvl;
            w.nature     = Lookup::getRandomNature(m_rng);

            const Poke *poke = Lookup::getPoke(roll.poke_id);
            int moveIndex = 0;
            for (int i = poke->eligible_move_count - 1; i >= 0 && moveIndex < 4; i--) {
                int moveLevel = poke->eligible_moves[i].level;
                if (moveLevel > 0 && moveLevel <= w.lvl) {
                    w.moves[moveIndex] = poke->eligible_moves[i].move_id;
                    moveIndex++;
                }
            }
            m_db.setWild(w);
        }

        m_wildPokemon = new WildPokemon(m_db.wild().pokedex_id, m_spawnPoint, m_spawnDirection);
        connect(m_wildPokemon, &WildPokemon::startABattle, this, &Game::handleBattleStart);
    }else{
        m_wildPokemon = new WildPokemon(m_db.party()[0].pokedex_id, m_spawnPoint, m_spawnDirection, true);
    }

    m_wildPokemon->show();
    m_spawnTimer->stop();
}

void Game::setRandomSpawnPoint(){
    const QRect screen = Globals::screenGeometry();
    std::uniform_int_distribution<int> distX(0, screen.width()-64*Globals::scale());
    std::uniform_int_distribution<int> distY(0, screen.height()-64*Globals::scale());
    m_spawnPoint = QPoint(distX(m_rng), distY(m_rng));
}


void Game::handleSaveSelected(int saveId){
    if(m_db.currentSaveId()==saveId) return;
    m_db.clearCache();
    m_db.setCurrentSaveId(saveId);
    setRandomSpawnPoint();
    resetGame();
    initializeGame();
}

void Game::deleteCurrentSave() {
    QMessageBox msgBox;
    msgBox.setWindowTitle("DesktopInvasion - Confirm Delete");
    const QString text = QString("Are you sure that you want to discard '") +
                     QString::fromStdString(m_db.loadGameState().name) +
                     QString("'? This cannot be undone.");
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setIcon(QMessageBox::Warning);

    if (msgBox.exec() != QMessageBox::Discard) {
        return;
    }

    m_db.deleteSave(m_db.currentSaveId());
    m_db.clearCache();
    auto v = m_db.listSaveIds();
    if (v.empty()) {
        m_db.setCurrentSaveId(0);
        resetGame();
        openStarterMenu();
    } else {
        m_db.setCurrentSaveId(v.back());

        m_trayIcon->m_gameActive = false;
        m_trayIcon->toggleGameActive();
        m_trayIcon->show();
        m_gameUsedToBeActive = true;

        resetGame();
        initializeGame();
        setGameActive(true);
    }
}





//Below method sends a QMap for each pokemon in party and box,
//with all the data needed for display, editing, and evolving.
//
//Some of this method must be duplicated in the
//"handleEvolveRequest" that is defined below.
//That is because I dont load all pokemon-evolution data in at menu build,
//but rather I send it as a signal once the evolve menu is opened.
//
//In retrospect, it might have just been cleaner to pass evolve data
//immediately so that there was no need for this ugly duplicate multi-step
//signal passing.... but hey at least now if you have 500 pokemon that are
//all able to evolve, there is no added load!
//k
//And don't tell me there's some kind of QContext/Q data view thingie
//that would have worked better, I have chosen for annoying multi-signal
//passes to plug up holes in my game, and I will god damn stand by it!
QVariantMap Game::pokemonToMenuState(int slot, const PokemonState& p) {
    QVariantMap entry;
    entry["slot"]   = slot;
    entry["pokedex_id"] = p.pokedex_id;
    entry["iconId"] = p.pokedex_id - 1;
    entry["name"]   = QString::fromStdString(p.name);
    entry["level"]  = p.lvl;
    entry["nature"] = QString::fromStdString(PokeTypes::natureToString(p.nature));
    entry["currentXP"] = p.currentXP;
    entry["requiredXP"] = PokeMath::xpToNextLevel(p.lvl);

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

    QVariantList eligibleEvolves;
    for(int i = 0; i < poke->eligible_evolve_count; i++){
        if(poke->eligible_evolves[i].level<=p.lvl){
            eligibleEvolves.append(poke->eligible_evolves[i].pokedex_id);
        }
    }
    entry["evolves"] = eligibleEvolves;

    std::vector<EligibleEntry> eligible_move;
    std::vector<EligibleEntry> eligible_tm_move;
    for (int i = 0; i < poke->eligible_move_count; i++) {
        int moveLevel = poke->eligible_moves[i].level;
        if (moveLevel == -1) {
            eligible_tm_move.push_back({ moveLevel, poke->eligible_moves[i].move_id });
        } else if (p.lvl >= moveLevel) {
            eligible_move.push_back({ moveLevel, poke->eligible_moves[i].move_id });
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
        moveData["category"] = static_cast<int>(mv->category);
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
        moveData["category"] = static_cast<int>(mv->category);
        moves.append(moveData);
    }
    entry["moves"] = moves;

    return entry;
}

//Below method contains the base pokemon's menu-relevant data,
//and that of its possible evolutions in "evolvesList"
//(See comment in above method "pokemonToMenuState")
void Game::handleEvolveRequest(int boxIndex, int slot, const QVariantMap& pokeData) {
    PokemonState originalState;
    if (boxIndex == -1) {
        originalState = m_db.party()[slot];
    } else {
        originalState = m_db.getBox(boxIndex)[slot];
    }

    QVariantList evolveIds = pokeData["evolves"].toList();
    QVariantList evolvesList;

    for (int i = 0; i < evolveIds.size(); i++) {
        int pokedexId     = evolveIds[i].toInt();
        const Poke* poke  = Lookup::getPoke(pokedexId);

        PokemonState p;
        p.pokedex_id  = pokedexId;
        p.name        = poke->name;
        p.pokeball_id = originalState.pokeball_id;
        p.lvl         = originalState.lvl;
        p.currentXP   = originalState.currentXP;
        p.nature      = originalState.nature;
        for (int m = 0; m < 4; m++)
            p.moves[m] = originalState.moves[m];
        QVariantMap entry = pokemonToMenuState(slot, p);
        entry["box"]  = boxIndex;
        entry["slot"] = slot;
        evolvesList.append(entry);
    }

    QVariantMap evolvesData;
    evolvesData["slot"]       = slot;
    evolvesData["box"]        = boxIndex;
    evolvesData["pokeName"]   = pokeData["pokeName"];
    evolvesData["currentXP"] = pokeData["currentXP"];
    evolvesData["requiredXP"] = pokeData["requiredXP"];
    evolvesData["name"]   = pokeData["name"];
    evolvesData["type1"] = pokeData["type1"];
    evolvesData["type2"] = pokeData["type2"];
    evolvesData["rowId"] = pokeData["rowId"];
    evolvesData["isBig"] = pokeData["isBig"];
    evolvesData["evolvesList"] = evolvesList;
    m_menu->updateEvolveMenu(evolvesData);
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

void Game::handleNameChange(int placex, int posx, const QString& name) {
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

void Game::writeDefaults() {
    Defaults d;
    d.scale        = Globals::scale();
    d.speed        = Globals::animationSpeed();
    d.lvlRangeUp   = Globals::encounterLvlHigh();
    d.lvlRangeDown = Globals::encounterLvlLow();
    d.expShareOn   = Globals::expShare();
    d.petModeOn    = m_petMode;
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
    std::string trainerName = m_db.loadGameState().name;
    auto battleMoveHandler = std::make_unique<BattleMoveHandler>(wildState, party, trainerName, m_rng);

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
            if (moveId < 1){
                if(m==0){
                    const Move* mv = Lookup::getMove(1);
                    battleParty.moves[slot][m] = {mv->name, PokeTypes::typeToString(mv->type)};
                }
                continue;
            }
            const Move* mv = Lookup::getMove(moveId);
            battleParty.moves[slot][m] = {mv->name, PokeTypes::typeToString(mv->type)};
        }
    }

    std::array<int, 3> ballCount = m_db.loadPokeballs();
    m_activeBattle = new Battle(m_spawnPoint, m_spawnDirection, wildState, battleParty, ballCount, trainerName, std::move(battleMoveHandler));

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

void Game::updatePartyXP(const std::array<int,6>& spread) {
    if (!m_activeBattle) return;

    std::array<int, 6> lvlUps = {-1, -1, -1, -1, -1, -1};
    std::array<QString,6> evolves = {"", "", "", "", "", ""};

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

                const Poke* poke = Lookup::getPoke(p.pokedex_id);
                if(poke->eligible_evolve_count>0 && p.lvl >= poke->eligible_evolves[0].level){
                    evolves[i] = Lookup::getPoke(poke->eligible_evolves[0].pokedex_id)->name;
                }
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


    m_db.changePokeball(m_ballGetCount, m_ballGetId);
    m_db.addTechnicalMove(m_tmGetId);

    m_activeBattle->showUpdateAndEndBattle(spread, lvlUps, evolves, m_tmGetId, m_ballGetCount, m_ballGetId);
}

void Game::handleEvolvePokemon(int boxIndex, int slot, int targetPokedexId, const std::string& nickName) {
    m_db.evolvePokemon(boxIndex, slot, targetPokedexId, nickName);

    if (boxIndex == -1) {
        m_menu->loadParty(partyToVariantList(), false);
    } else {
        pushBoxToMenu(boxIndex);
        m_menu->loadParty(partyToVariantList(), false);
        m_menu->showBox(boxIndex);
    }
}

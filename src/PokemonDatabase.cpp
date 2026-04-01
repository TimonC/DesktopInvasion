#include "PokemonDatabase.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <cassert>
#include <unordered_set>

#define DB_LOG(msg)  qDebug().nospace()    << "[DB] " << msg
#define DB_WARN(msg) qWarning().nospace()  << "[DB] " << msg
#define DB_ERR(msg)  qCritical().nospace() << "[DB] " << msg

static void logQuery(const QSqlQuery& q) {
    if (q.lastError().isValid())
        qCritical().nospace() << "[DB] Query failed: " << q.lastError().text() << " | SQL: " << q.lastQuery();
}

PokemonDatabase& PokemonDatabase::instance() {
    static PokemonDatabase inst;
    return inst;
}

void PokemonDatabase::clearCache() {
    m_wild = PokemonState();
    for (int i = 0; i < PARTY_SIZE; ++i) {
        m_party[i] = PokemonState();
    }
    m_boxCache.clear();
}

PokemonDatabase::~PokemonDatabase() { shutdown(); }

int PokemonDatabase::initialize() {
    if (m_initialized) {
        DB_WARN("initialize called but already initialized");
        return 1;
    }

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        DB_ERR("SQLITE driver not available");
        return -1;
    }

    QString basePath;

#ifdef Q_OS_WIN
    basePath = QCoreApplication::applicationDirPath() + "/db";
    DB_LOG("Windows mode — using local db folder: " << basePath);
#else
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    basePath = appDataLocation;
    if (basePath.isEmpty()) {
        basePath = QCoreApplication::applicationDirPath();
        DB_WARN("AppDataLocation empty, falling back to applicationDirPath");
    }
#endif

    QDir dbDir(basePath);
    if (!dbDir.exists() && !dbDir.mkpath(basePath)) {
        DB_ERR("Failed to create database directory: " << dbDir.path());
        return -1;
    }

    QString path = dbDir.filePath("app.db");
    DB_LOG("Initializing — path:" << path);

    QFile testFile(dbDir.filePath("test.tmp"));
    if (!testFile.open(QIODevice::WriteOnly)) {
        DB_ERR("Database directory not writable: " << dbDir.path() << " - " << testFile.errorString());
        return -1;
    }
    testFile.remove();

    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

    if (!db.open()) {
        DB_ERR("Failed to open DB: " << db.lastError().text());
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
        return -1;
    }

    m_dbPath = path;
    m_initialized = true;

    if (!createTables()) {
        DB_ERR("Failed to create tables");
        shutdown();
        return -1;
    }

    {
        QSqlQuery cleanup;
        if (!cleanup.exec("DELETE FROM saves WHERE save_id NOT IN (SELECT save_id FROM save_list)")) {
            DB_WARN("Failed to cleanup orphaned saves rows");
            logQuery(cleanup);
        } else {
            DB_LOG("Cleaned up orphaned saves rows");
        }
    }

    m_saveId = readCurrentSaveId();
    if (m_saveId == 0)
        return 0;

    if (!initFixedSlots()) {
        DB_ERR("Failed to initialize fixed slots");
        shutdown();
        return -1;
    }

    if (!loadWildAndParty()) {
        DB_ERR("Failed to load wild and party");
        shutdown();
        return -1;
    }

    DB_LOG("Initialization complete");
    return 1;
}

void PokemonDatabase::shutdown() {
    if (!m_initialized) return;
    DB_LOG("Shutdown");

    {
        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) db.close();
    }

    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

    m_initialized = false;
}

bool PokemonDatabase::createTables() {
    DB_LOG("Creating tables if not exists");
    QSqlQuery q;
    bool ok = true;
    auto run = [&](const char* sql) { if (!q.exec(sql)) { logQuery(q); ok = false; } };

    run(R"(CREATE TABLE IF NOT EXISTS save_id_counter (
        id      INTEGER PRIMARY KEY CHECK(id = 1),
        counter INTEGER DEFAULT 0
    ))");
    run(R"(CREATE TABLE IF NOT EXISTS save_list (
        save_id INTEGER PRIMARY KEY
    ))");
    run(R"(CREATE TABLE IF NOT EXISTS current_save (
        id              INTEGER PRIMARY KEY CHECK(id = 1),
        current_save_id INTEGER DEFAULT 0
    ))");
    run(R"(CREATE TABLE IF NOT EXISTS saves (
        save_id          INTEGER PRIMARY KEY,
        player_sprite_id INTEGER DEFAULT 0,
        name             TEXT    DEFAULT 'Player',
        current_box      INTEGER DEFAULT 0,
        unlocked_boxes   INTEGER DEFAULT 10
    ))");
    run(R"(CREATE TABLE IF NOT EXISTS wild_slot (
        save_id       INTEGER PRIMARY KEY,
        pokedex_id    INTEGER DEFAULT 0,
        variant_id    INTEGER DEFAULT 0,
        pokeball_id   INTEGER DEFAULT 0,
        name          TEXT    DEFAULT '',
        lvl           INTEGER DEFAULT 1,
        current_xp    INTEGER DEFAULT 0,
        nature        INTEGER DEFAULT 0,
        move0         INTEGER DEFAULT 0,
        move1         INTEGER DEFAULT 0,
        move2         INTEGER DEFAULT 0,
        move3         INTEGER DEFAULT 0,
        FOREIGN KEY(save_id) REFERENCES saves(save_id)
    ))");
    run(R"(CREATE TABLE IF NOT EXISTS party_slots (
        save_id       INTEGER NOT NULL,
        slot          INTEGER NOT NULL,
        pokedex_id    INTEGER DEFAULT 0,
        variant_id    INTEGER DEFAULT 0,
        pokeball_id   INTEGER DEFAULT 0,
        name          TEXT    DEFAULT '',
        lvl           INTEGER DEFAULT 1,
        current_xp    INTEGER DEFAULT 0,
        nature        INTEGER DEFAULT 0,
        move0         INTEGER DEFAULT 0,
        move1         INTEGER DEFAULT 0,
        move2         INTEGER DEFAULT 0,
        move3         INTEGER DEFAULT 0,
        PRIMARY KEY(save_id, slot),
        FOREIGN KEY(save_id) REFERENCES saves(save_id)
    ))");
    run(R"(CREATE TABLE IF NOT EXISTS pc_slots (
        save_id       INTEGER NOT NULL,
        box           INTEGER NOT NULL,
        slot          INTEGER NOT NULL,
        pokedex_id    INTEGER DEFAULT 0,
        variant_id    INTEGER DEFAULT 0,
        pokeball_id   INTEGER DEFAULT 0,
        name          TEXT    DEFAULT '',
        lvl           INTEGER DEFAULT 1,
        current_xp    INTEGER DEFAULT 0,
        nature        INTEGER DEFAULT 0,
        move0         INTEGER DEFAULT 0,
        move1         INTEGER DEFAULT 0,
        move2         INTEGER DEFAULT 0,
        move3         INTEGER DEFAULT 0,
        PRIMARY KEY(save_id, box, slot),
        FOREIGN KEY(save_id) REFERENCES saves(save_id)
    ))");
    run(R"(CREATE TABLE IF NOT EXISTS defaults (
        save_id        INTEGER PRIMARY KEY,
        scale          INTEGER DEFAULT 2,
        speed          INTEGER DEFAULT 1,
        lvl_range_up   INTEGER DEFAULT 5,
        lvl_range_down INTEGER DEFAULT 5,
        exp_share_on   INTEGER DEFAULT 0,
        pet_mode_on    INTEGER DEFAULT 0,
        FOREIGN KEY(save_id) REFERENCES saves(save_id)
    ))");
    run(R"(CREATE TABLE IF NOT EXISTS pokeballs (
        save_id INTEGER NOT NULL,
        row     INTEGER NOT NULL,
        count   INTEGER DEFAULT 0,
        PRIMARY KEY(save_id, row),
        FOREIGN KEY(save_id) REFERENCES saves(save_id)
    ))");
    run(R"(CREATE TABLE IF NOT EXISTS technical_moves (
        save_id INTEGER NOT NULL,
        move_id INTEGER NOT NULL,
        PRIMARY KEY(save_id, move_id),
        FOREIGN KEY(save_id) REFERENCES saves(save_id)
    ))");
    run("CREATE INDEX IF NOT EXISTS idx_pc_box ON pc_slots(save_id, box)");

    run("INSERT OR IGNORE INTO save_id_counter(id, counter) VALUES(1, 0)");
    run("INSERT OR IGNORE INTO current_save(id, current_save_id) VALUES(1, 0)");

    {
        QSqlQuery alter(QSqlDatabase::database());
        alter.prepare("ALTER TABLE defaults ADD COLUMN pet_mode_on INTEGER DEFAULT 0");
        alter.exec();
    }

    if (ok) DB_LOG("Tables ready");
    else    DB_ERR("One or more tables failed to create");
    return ok;
}

int PokemonDatabase::readCurrentSaveId() {
    QSqlQuery q;
    q.prepare("SELECT current_save_id FROM current_save WHERE id=1");
    if (q.exec() && q.next())
        return q.value(0).toInt();
    return 0;
}

bool PokemonDatabase::writeCurrentSaveId(int save_id) {
    QSqlQuery q;
    q.prepare("UPDATE current_save SET current_save_id=? WHERE id=1");
    q.addBindValue(save_id);
    bool ok = q.exec();
    if (!ok) logQuery(q);
    else DB_LOG("writeCurrentSaveId: current_save_id=" << save_id);
    return ok;
}

bool PokemonDatabase::setCurrentSaveId(int save_id) {
    DB_LOG("setCurrentSaveId: switching to save_id=" << save_id);
    if (!writeCurrentSaveId(save_id)) return false;
    m_saveId    = save_id;
    m_boxCache.clear();
    if (!initFixedSlots())   { DB_ERR("setCurrentSaveId: failed to initialize fixed slots"); return false; }
    if (!loadWildAndParty()) { DB_ERR("setCurrentSaveId: failed to reload wild/party");      return false; }
    DB_LOG("setCurrentSaveId: active save is now " << m_saveId);
    return true;
}

int PokemonDatabase::nextSaveIdFromCounter() {
    QSqlQuery q;
    q.prepare("UPDATE save_id_counter SET counter = counter + 1 WHERE id = 1");
    if (!q.exec()) { logQuery(q); return -1; }
    q.prepare("SELECT counter FROM save_id_counter WHERE id = 1");
    if (q.exec() && q.next()) {
        int next = q.value(0).toInt();
        DB_LOG("nextSaveIdFromCounter: new counter=" << next);
        return next;
    }
    DB_ERR("nextSaveIdFromCounter: failed to read counter after increment");
    logQuery(q);
    return -1;
}

int PokemonDatabase::createNewSave(const GameState& stateIn, const PokemonState& starter) {
    DB_LOG("createNewSave: allocating new save id");

    int newId = nextSaveIdFromCounter();
    if (newId < 1) { DB_ERR("createNewSave: counter increment failed"); return -1; }

    QSqlQuery q;
    q.prepare("INSERT OR IGNORE INTO save_list(save_id) VALUES(?)");
    q.addBindValue(newId);
    if (!q.exec()) { logQuery(q); return -1; }

    GameState state  = stateIn;
    state.save_id    = newId;

    if (!saveGameState(state)) { DB_ERR("createNewSave: saveGameState failed"); return -1; }
    if (!writeCurrentSaveId(newId)) { DB_ERR("createNewSave: writeCurrentSaveId failed"); return -1; }

    m_saveId = newId;
    m_boxCache.clear();
    m_wild   = PokemonState{};
    m_party  = {};

    if (!initFixedSlots())   { DB_ERR("createNewSave: initFixedSlots failed");   return -1; }
    if (!loadWildAndParty()) { DB_ERR("createNewSave: loadWildAndParty failed"); return -1; }

    setPartySlot(0, starter);

    DB_LOG("createNewSave: new save_id=" << newId << " active, starter=" << QString::fromStdString(starter.name));
    return newId;
}

bool PokemonDatabase::deleteSave(int save_id) {
    DB_LOG("deleteSave: save_id=" << save_id);
    QSqlQuery q;
    bool ok = true;
    auto del = [&](const QString& sql) {
        q.prepare(sql);
        q.addBindValue(save_id);
        if (!q.exec()) { logQuery(q); ok = false; }
    };

    del("DELETE FROM pc_slots        WHERE save_id=?");
    del("DELETE FROM party_slots     WHERE save_id=?");
    del("DELETE FROM wild_slot       WHERE save_id=?");
    del("DELETE FROM defaults        WHERE save_id=?");
    del("DELETE FROM pokeballs       WHERE save_id=?");
    del("DELETE FROM technical_moves WHERE save_id=?");
    del("DELETE FROM saves           WHERE save_id=?");
    del("DELETE FROM save_list       WHERE save_id=?");

    if (save_id == m_saveId) {
        DB_WARN("deleteSave: deleted the active save — caller must switch to another save");
        m_saveId = 0;
        m_boxCache.clear();
        m_wild   = PokemonState{};
        m_party  = {};
        writeCurrentSaveId(0);
    }

    if (ok) DB_LOG("deleteSave: save_id=" << save_id << " removed");
    else    DB_ERR("deleteSave: one or more deletes failed for save_id=" << save_id);
    return ok;
}

std::vector<int> PokemonDatabase::listSaveIds() {
    std::vector<int> ids;
    QSqlQuery q;
    q.prepare("SELECT save_id FROM save_list ORDER BY save_id ASC");
    if (q.exec()) {
        while (q.next()) ids.push_back(q.value(0).toInt());
    } else {
        logQuery(q);
    }
    DB_LOG("listSaveIds: " << ids.size() << " saves");
    return ids;
}

bool PokemonDatabase::initFixedSlots() {
    QSqlQuery check;
    check.prepare("SELECT 1 FROM save_list WHERE save_id = ?");
    check.addBindValue(m_saveId);

    if (!(check.exec() && check.next()))
        return false;

    QSqlQuery q;

    q.prepare("INSERT OR IGNORE INTO saves(save_id) VALUES(?)");
    q.addBindValue(m_saveId);
    if (!q.exec()) return false;

    q.prepare("INSERT OR IGNORE INTO wild_slot(save_id) VALUES(?)");
    q.addBindValue(m_saveId);
    if (!q.exec()) return false;

    q.prepare("INSERT OR IGNORE INTO defaults(save_id) VALUES(?)");
    q.addBindValue(m_saveId);
    if (!q.exec()) return false;

    q.prepare("INSERT OR IGNORE INTO party_slots(save_id, slot) VALUES(?, ?)");
    for (int i = 0; i < PARTY_SIZE; ++i) {
        q.addBindValue(m_saveId);
        q.addBindValue(i);
        if (!q.exec()) return false;
    }

    q.prepare("INSERT OR IGNORE INTO pokeballs(save_id, row, count) VALUES(?, ?, 0)");
    for (int r = 0; r < 3; ++r) {
        q.addBindValue(m_saveId);
        q.addBindValue(r);
        if (!q.exec()) return false;
    }

    return true;
}

PokemonState PokemonDatabase::rowToPokemon(const QSqlQuery& q) {
    PokemonState p;
    p.pokedex_id  = q.value("pokedex_id").toInt();
    p.variant_id  = q.value("variant_id").toInt();
    p.pokeball_id = q.value("pokeball_id").toInt();
    p.name        = q.value("name").toString().toStdString();
    p.lvl         = q.value("lvl").toInt();
    p.currentXP   = q.value("current_xp").toInt();
    p.nature      = static_cast<Nature>(q.value("nature").toInt());
    p.moves[0]    = q.value("move0").toInt();
    p.moves[1]    = q.value("move1").toInt();
    p.moves[2]    = q.value("move2").toInt();
    p.moves[3]    = q.value("move3").toInt();
    return p;
}

void PokemonDatabase::writePokemonToRow(QSqlQuery& q, const PokemonState& p) {
    q.addBindValue(p.pokedex_id);
    q.addBindValue(p.variant_id);
    q.addBindValue(p.pokeball_id);
    q.addBindValue(QString::fromStdString(p.name));
    q.addBindValue(p.lvl);
    q.addBindValue(p.currentXP);
    q.addBindValue(static_cast<int>(p.nature));
    for (int i = 0; i < 4; ++i) q.addBindValue(p.moves[i]);
}

bool PokemonDatabase::loadWildAndParty() {
    DB_LOG("Loading wild and party for save_id=" << m_saveId);
    QSqlQuery q;
    bool ok = true;

    q.prepare("SELECT * FROM wild_slot WHERE save_id=?");
    q.addBindValue(m_saveId);
    if (q.exec()) {
        if (q.next()) {
            m_wild = rowToPokemon(q);
            DB_LOG("Wild: " << (m_wild.empty() ? "empty" : QString::fromStdString(m_wild.name)));
        } else {
            DB_WARN("No wild_slot row found for save_id=" << m_saveId);
        }
    } else { logQuery(q); ok = false; }

    q.prepare("SELECT * FROM party_slots WHERE save_id=? ORDER BY slot");
    q.addBindValue(m_saveId);
    if (q.exec()) {
        int count = 0;
        while (q.next()) {
            int slot      = q.value("slot").toInt();
            m_party[slot] = rowToPokemon(q);
            if (!m_party[slot].empty()) ++count;
        }
        DB_LOG("Party: " << count << "/6 slots occupied");
    } else { logQuery(q); ok = false; }

    return ok;
}

bool PokemonDatabase::dbWriteWild(const PokemonState& p) {
    QSqlQuery q;
    q.prepare(R"(UPDATE wild_slot SET
        pokedex_id=?, variant_id=?, pokeball_id=?, name=?,
        lvl=?, current_xp=?, nature=?,
        move0=?, move1=?, move2=?, move3=?
        WHERE save_id=?)");
    writePokemonToRow(q, p);
    q.addBindValue(m_saveId);
    bool ok = q.exec();
    if (!ok) logQuery(q);
    else DB_LOG("Wild -> " << (p.empty() ? "empty" : QString::fromStdString(p.name)));
    return ok;
}

bool PokemonDatabase::dbWritePartySlot(int slot, const PokemonState& p) {
    QSqlQuery q;
    q.prepare(R"(UPDATE party_slots SET
        pokedex_id=?, variant_id=?, pokeball_id=?, name=?,
        lvl=?, current_xp=?, nature=?,
        move0=?, move1=?, move2=?, move3=?
        WHERE save_id=? AND slot=?)");
    writePokemonToRow(q, p);
    q.addBindValue(m_saveId);
    q.addBindValue(slot);
    bool ok = q.exec();
    if (!ok) logQuery(q);
    else DB_LOG("Party[" << slot << "] -> " << (p.empty() ? "empty" : QString::fromStdString(p.name)));
    return ok;
}

bool PokemonDatabase::dbWritePCSlot(int box, int slot, const PokemonState& p) {
    if (p.empty()) return dbDeletePCSlot(box, slot);
    QSqlQuery q;
    q.prepare(R"(INSERT OR REPLACE INTO pc_slots
        (save_id, box, slot, pokedex_id, variant_id, pokeball_id, name,
         lvl, current_xp, nature, move0, move1, move2, move3)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?))");
    q.addBindValue(m_saveId);
    q.addBindValue(box);
    q.addBindValue(slot);
    writePokemonToRow(q, p);
    bool ok = q.exec();
    if (!ok) logQuery(q);
    else DB_LOG("PC[" << box << "][" << slot << "] -> " << QString::fromStdString(p.name));
    return ok;
}

bool PokemonDatabase::dbDeletePCSlot(int box, int slot) {
    QSqlQuery q;
    q.prepare("DELETE FROM pc_slots WHERE save_id=? AND box=? AND slot=?");
    q.addBindValue(m_saveId);
    q.addBindValue(box);
    q.addBindValue(slot);
    bool ok = q.exec();
    if (!ok) logQuery(q);
    else DB_LOG("PC[" << box << "][" << slot << "] -> cleared");
    return ok;
}

void PokemonDatabase::setWild(const PokemonState& p) {
    DB_LOG("setWild: " << (p.empty() ? "empty" : QString::fromStdString(p.name)));
    m_wild = p;
    dbWriteWild(p);
}

void PokemonDatabase::clearWild() {
    DB_LOG("clearWild");
    m_wild = PokemonState{};
    dbWriteWild(m_wild);
}

void PokemonDatabase::setPartySlot(int slot, const PokemonState& p) {
    assert(slot >= 0 && slot < PARTY_SIZE);
    DB_LOG("setPartySlot[" << slot << "]: " << (p.empty() ? "empty" : QString::fromStdString(p.name)));
    m_party[slot] = p;
    dbWritePartySlot(slot, p);
}

int PokemonDatabase::firstFreePartySlot() const {
    for (int i = 0; i < PARTY_SIZE; ++i)
        if (m_party[i].empty()) return i;
    DB_LOG("firstFreePartySlot: party full");
    return -1;
}

int PokemonDatabase::partySize() const {
    int n = 0;
    for (const auto& p : m_party) if (!p.empty()) ++n;
    return n;
}

bool PokemonDatabase::loadBox(int box) {
    if (m_boxCache.count(box)) return true;

    DB_LOG("Loading PC box " << box);
    QSqlQuery q;
    q.prepare("SELECT * FROM pc_slots WHERE save_id=? AND box=?");
    q.addBindValue(m_saveId);
    q.addBindValue(box);
    if (!q.exec()) { logQuery(q); return false; }

    std::array<PokemonState, BOX_SIZE> arr{};
    int count = 0;
    while (q.next()) {
        int slot  = q.value("slot").toInt();
        arr[slot] = rowToPokemon(q);
        ++count;
    }
    m_boxCache[box] = arr;
    DB_LOG("PC box " << box << " loaded — " << count << " pokemon");
    return true;
}

bool PokemonDatabase::isBoxLoaded(int box) const {
    return m_boxCache.count(box) > 0;
}

const std::array<PokemonState, BOX_SIZE>& PokemonDatabase::getBox(int box) const {
    assert(isBoxLoaded(box));
    return m_boxCache.at(box);
}

bool PokemonDatabase::setPCSlot(int box, int slot, const PokemonState& p) {
    DB_LOG("setPCSlot[" << box << "][" << slot << "]: " << (p.empty() ? "empty" : QString::fromStdString(p.name)));
    if (!isBoxLoaded(box) && !loadBox(box)) {
        DB_ERR("setPCSlot: failed to load box " << box);
        return false;
    }
    m_boxCache[box][slot] = p;
    return dbWritePCSlot(box, slot, p);
}

std::pair<int, int> PokemonDatabase::firstFreePC() {
    GameState gs = loadGameState();
    int boxLimit = gs.unlocked_boxes;
    DB_LOG("firstFreePC: scanning " << boxLimit << " boxes");

    for (int box = 0; box < boxLimit; ++box) {
        if (!loadBox(box)) { DB_WARN("firstFreePC: failed to load box " << box << " — skipping"); continue; }
        for (int slot = 0; slot < BOX_SIZE; ++slot) {
            if (m_boxCache[box][slot].empty()) {
                DB_LOG("firstFreePC: found free slot at [" << box << "][" << slot << "]");
                return {box, slot};
            }
        }
    }
    DB_WARN("firstFreePC: no free slots found across " << boxLimit << " boxes");
    return {-1, -1};
}

std::pair<int, int> PokemonDatabase::catchWildPokemon(int pokeball_id) {
    if (m_wild.empty()) {
        DB_WARN("catchWildPokemon: wild slot is empty, nothing to catch");
        return {-1, -1};
    }

    DB_LOG("catchWildPokemon: " << QString::fromStdString(m_wild.name) << " ball_id=" << pokeball_id);

    PokemonState caught = m_wild;
    caught.pokeball_id  = pokeball_id;

    int partySlot = firstFreePartySlot();
    if (partySlot >= 0) {
        setPartySlot(partySlot, caught);
        clearWild();
        DB_LOG("Caught " << QString::fromStdString(caught.name) << " -> party[" << partySlot << "]");
        return {-1, partySlot};
    }

    auto [box, slot] = firstFreePC();
    if (box < 0) {
        DB_WARN("catchWildPokemon: storage full, could not catch " << QString::fromStdString(caught.name));
        return {-1, -1};
    }
    if (!setPCSlot(box, slot, caught)) {
        DB_ERR("catchWildPokemon: failed to write " << QString::fromStdString(caught.name) << " to PC[" << box << "][" << slot << "]");
        return {-1, -1};
    }
    clearWild();
    DB_LOG("Caught " << QString::fromStdString(caught.name) << " -> PC[" << box << "][" << slot << "]");
    return {box, slot};
}

GameState PokemonDatabase::loadGameState() {
    GameState state;
    state.save_id = m_saveId;
    QSqlQuery q;
    q.prepare("SELECT * FROM saves WHERE save_id=?");
    q.addBindValue(m_saveId);
    if (q.exec() && q.next()) {
        state.player_sprite_id = q.value("player_sprite_id").toInt();
        state.name             = q.value("name").toString().toStdString();
        state.current_box      = q.value("current_box").toInt();
        state.unlocked_boxes   = q.value("unlocked_boxes").toInt();
        DB_LOG("loadGameState: name=" << QString::fromStdString(state.name)
               << " current_box=" << state.current_box
               << " unlocked_boxes=" << state.unlocked_boxes);
    } else {
        DB_WARN("loadGameState: no row for save_id=" << m_saveId << " — writing defaults");
        logQuery(q);
        saveGameState(state);
    }
    return state;
}

bool PokemonDatabase::saveGameState(const GameState& state) {
    DB_LOG("saveGameState: save_id=" << state.save_id << " name=" << QString::fromStdString(state.name));
    QSqlQuery q;
    q.prepare(R"(INSERT OR REPLACE INTO saves
        (save_id, player_sprite_id, name, current_box, unlocked_boxes)
        VALUES(?,?,?,?,?))");
    q.addBindValue(state.save_id);
    q.addBindValue(state.player_sprite_id);
    q.addBindValue(QString::fromStdString(state.name));
    q.addBindValue(state.current_box);
    q.addBindValue(state.unlocked_boxes);
    bool ok = q.exec();
    if (!ok) logQuery(q);
    else DB_LOG("GameState saved (save_id=" << state.save_id << ")");
    return ok;
}

bool PokemonDatabase::swapByPos(int boxX, int slotX, int boxY, int slotY) {
    if (boxX == boxY && slotX == slotY) {
        DB_LOG("swapByPos: same position [" << boxX << "][" << slotX << "] — no-op");
        return true;
    }

    DB_LOG("swapByPos: [" << boxX << "][" << slotX << "] <-> [" << boxY << "][" << slotY << "]");

    const bool xIsParty = (boxX == -1);
    const bool yIsParty = (boxY == -1);

    if (xIsParty && yIsParty) {
        std::swap(m_party[slotX], m_party[slotY]);
        bool ok1 = dbWritePartySlot(slotX, m_party[slotX]);
        bool ok2 = dbWritePartySlot(slotY, m_party[slotY]);
        if (!ok1 || !ok2) DB_ERR("swapByPos: party <-> party write failed");
        return ok1 & ok2;
    }

    if (!xIsParty && !yIsParty) {
        if (!isBoxLoaded(boxX) && !loadBox(boxX)) { DB_ERR("swapByPos: failed to load box " << boxX); return false; }
        if (!isBoxLoaded(boxY) && !loadBox(boxY)) { DB_ERR("swapByPos: failed to load box " << boxY); return false; }
        std::swap(m_boxCache[boxX][slotX], m_boxCache[boxY][slotY]);
        bool ok1 = dbWritePCSlot(boxX, slotX, m_boxCache[boxX][slotX]);
        bool ok2 = dbWritePCSlot(boxY, slotY, m_boxCache[boxY][slotY]);
        if (!ok1 || !ok2) DB_ERR("swapByPos: PC <-> PC write failed");
        return ok1 && ok2;
    }

    const int pcBox  = xIsParty ? boxY  : boxX;
    const int pcSlot = xIsParty ? slotY : slotX;
    const int pSlot  = xIsParty ? slotX : slotY;
    if (!isBoxLoaded(pcBox) && !loadBox(pcBox)) { DB_ERR("swapByPos: failed to load box " << pcBox); return false; }
    std::swap(m_boxCache[pcBox][pcSlot], m_party[pSlot]);
    bool ok1 = dbWritePCSlot(pcBox, pcSlot, m_boxCache[pcBox][pcSlot]);
    bool ok2 = dbWritePartySlot(pSlot, m_party[pSlot]);
    if (!ok1 || !ok2) DB_ERR("swapByPos: party <-> PC write failed");
    return ok1 & ok2;
}

bool PokemonDatabase::toggleExpShare() {
    Defaults d = loadDefaults();
    d.expShareOn = !d.expShareOn;
    DB_LOG("toggleExpShare -> " << (d.expShareOn ? "ON" : "OFF"));
    return writeDefaults(d);
}

bool PokemonDatabase::isExpShareOn() const {
    return const_cast<PokemonDatabase*>(this)->loadDefaults().expShareOn;
}

PokemonState* PokemonDatabase::cachePtr(int box, int slot) {
    if (box == -1) {
        if (slot < 0 || slot >= PARTY_SIZE) {
            DB_WARN("cachePtr: party slot " << slot << " out of range");
            return nullptr;
        }
        return &m_party[slot];
    }
    if (!isBoxLoaded(box) && !loadBox(box)) {
        DB_ERR("cachePtr: failed to load box " << box);
        return nullptr;
    }
    if (slot < 0 || slot >= BOX_SIZE) {
        DB_WARN("cachePtr: PC slot " << slot << " out of range in box " << box);
        return nullptr;
    }
    return &m_boxCache[box][slot];
}

bool PokemonDatabase::renamePokemon(int box, int slot, const std::string& newName) {
    DB_LOG("renamePokemon [box=" << box << " slot=" << slot << "] -> " << QString::fromStdString(newName));
    PokemonState* p = cachePtr(box, slot);
    if (!p)         { DB_WARN("renamePokemon: invalid slot [box=" << box << " slot=" << slot << "]"); return false; }
    if (p->empty()) { DB_WARN("renamePokemon: slot empty [box=" << box << " slot=" << slot << "]");  return false; }
    p->name = newName;

    QSqlQuery q;
    QString qName = QString::fromStdString(newName);
    if (box == -1) {
        q.prepare("UPDATE party_slots SET name=? WHERE save_id=? AND slot=?");
        q.addBindValue(qName); q.addBindValue(m_saveId); q.addBindValue(slot);
    } else {
        q.prepare("UPDATE pc_slots SET name=? WHERE save_id=? AND box=? AND slot=?");
        q.addBindValue(qName); q.addBindValue(m_saveId); q.addBindValue(box); q.addBindValue(slot);
    }
    bool ok = q.exec();
    if (!ok) logQuery(q);
    return ok;
}

bool PokemonDatabase::setPokemonMoves(int box, int slot, const int moves[4]) {
    DB_LOG("setPokemonMoves [box=" << box << " slot=" << slot << "] moves: "
           << moves[0] << ", " << moves[1] << ", " << moves[2] << ", " << moves[3]);
    PokemonState* p = cachePtr(box, slot);
    if (!p)         { DB_WARN("setPokemonMoves: invalid slot [box=" << box << " slot=" << slot << "]"); return false; }
    if (p->empty()) { DB_WARN("setPokemonMoves: slot empty [box=" << box << " slot=" << slot << "]");  return false; }
    for (int i = 0; i < 4; ++i) p->moves[i] = moves[i];

    QSqlQuery q;
    auto bind = [&]() {
        q.addBindValue(moves[0]); q.addBindValue(moves[1]);
        q.addBindValue(moves[2]); q.addBindValue(moves[3]);
    };
    if (box == -1) {
        q.prepare("UPDATE party_slots SET move0=?, move1=?, move2=?, move3=? WHERE save_id=? AND slot=?");
        bind(); q.addBindValue(m_saveId); q.addBindValue(slot);
    } else {
        q.prepare("UPDATE pc_slots SET move0=?, move1=?, move2=?, move3=? WHERE save_id=? AND box=? AND slot=?");
        bind(); q.addBindValue(m_saveId); q.addBindValue(box); q.addBindValue(slot);
    }
    bool ok = q.exec();
    if (!ok) logQuery(q);
    return ok;
}

bool PokemonDatabase::setPokemonMove(int box, int slot, int moveIndex, int moveId) {
    if (moveIndex < 0 || moveIndex > 3) {
        DB_WARN("setPokemonMove: moveIndex " << moveIndex << " out of range");
        return false;
    }
    DB_LOG("setPokemonMove [box=" << box << " slot=" << slot << "] move[" << moveIndex << "] -> " << moveId);
    PokemonState* p = cachePtr(box, slot);
    if (!p)         { DB_WARN("setPokemonMove: invalid slot [box=" << box << " slot=" << slot << "]"); return false; }
    if (p->empty()) { DB_WARN("setPokemonMove: slot empty [box=" << box << " slot=" << slot << "]");  return false; }
    p->moves[moveIndex] = moveId;

    static const char* cols[] = {"move0", "move1", "move2", "move3"};
    QSqlQuery q;
    if (box == -1) {
        q.prepare(QString("UPDATE party_slots SET %1=? WHERE save_id=? AND slot=?").arg(cols[moveIndex]));
        q.addBindValue(moveId); q.addBindValue(m_saveId); q.addBindValue(slot);
    } else {
        q.prepare(QString("UPDATE pc_slots SET %1=? WHERE save_id=? AND box=? AND slot=?").arg(cols[moveIndex]));
        q.addBindValue(moveId); q.addBindValue(m_saveId); q.addBindValue(box); q.addBindValue(slot);
    }
    bool ok = q.exec();
    if (!ok) logQuery(q);
    return ok;
}

Defaults PokemonDatabase::loadDefaults() {
    Defaults d;
    QSqlQuery q;
    q.prepare("SELECT scale, speed, lvl_range_up, lvl_range_down, exp_share_on, pet_mode_on FROM defaults WHERE save_id=?");
    q.addBindValue(m_saveId);
    if (q.exec() && q.next()) {
        d.scale        = q.value("scale").toInt();
        d.speed        = q.value("speed").toInt();
        d.lvlRangeUp   = q.value("lvl_range_up").toInt();
        d.lvlRangeDown = q.value("lvl_range_down").toInt();
        d.expShareOn   = q.value("exp_share_on").toBool();
        d.petModeOn    = q.value("pet_mode_on").toBool();
        DB_LOG("loadDefaults: scale=" << d.scale << " speed=" << d.speed
               << " lvlUp=" << d.lvlRangeUp << " lvlDown=" << d.lvlRangeDown
               << " expShare=" << d.expShareOn << " petMode=" << d.petModeOn);
    } else {
        DB_WARN("loadDefaults: no row for save_id=" << m_saveId << " — returning defaults");
        logQuery(q);
    }
    return d;
}

bool PokemonDatabase::writeDefaults(const Defaults& d) {
    DB_LOG("writeDefaults: scale=" << d.scale << " speed=" << d.speed
           << " lvlUp=" << d.lvlRangeUp << " lvlDown=" << d.lvlRangeDown
           << " expShare=" << d.expShareOn << " petMode=" << d.petModeOn);
    QSqlQuery q;
    q.prepare(R"(UPDATE defaults SET
        scale=?, speed=?, lvl_range_up=?, lvl_range_down=?, exp_share_on=?, pet_mode_on=?
        WHERE save_id=?)");
    q.addBindValue(d.scale);
    q.addBindValue(d.speed);
    q.addBindValue(d.lvlRangeUp);
    q.addBindValue(d.lvlRangeDown);
    q.addBindValue(d.expShareOn ? 1 : 0);
    q.addBindValue(d.petModeOn  ? 1 : 0);
    q.addBindValue(m_saveId);
    bool ok = q.exec();
    if (!ok) logQuery(q);
    return ok;
}

std::vector<std::pair<int, std::string>> PokemonDatabase::listTrainerNames() {
    std::vector<std::pair<int, std::string>> names;
    QSqlQuery q;
    q.prepare(R"(SELECT s.save_id, s.name FROM saves s
                 INNER JOIN save_list sl ON sl.save_id = s.save_id
                 ORDER BY s.save_id ASC)");
    if (q.exec()) {
        while (q.next())
            names.emplace_back(q.value(0).toInt(), q.value(1).toString().toStdString());
    } else {
        logQuery(q);
    }
    return names;
}

std::array<int, 3> PokemonDatabase::loadPokeballs() {
    std::array<int, 3> counts{0, 0, 0};
    QSqlQuery q;
    q.prepare("SELECT row, count FROM pokeballs WHERE save_id=? ORDER BY row ASC");
    q.addBindValue(m_saveId);
    if (q.exec()) {
        while (q.next()) {
            int r = q.value(0).toInt();
            if (r >= 0 && r < 3) counts[r] = q.value(1).toInt();
        }
    } else {
        logQuery(q);
    }
    DB_LOG("loadPokeballs: [" << counts[0] << ", " << counts[1] << ", " << counts[2] << "]");
    return counts;
}

bool PokemonDatabase::changePokeball(int delta, int row) {
    if (row < 1 || row > 3) {
        DB_WARN("changePokeball: row " << row << " out of range (1-3)");
        return false;
    }
    int zeroRow = row - 1;
    QSqlQuery q;
    q.prepare("UPDATE pokeballs SET count = MIN(999, MAX(0, count + ?)) WHERE save_id=? AND row=?");
    q.addBindValue(delta);
    q.addBindValue(m_saveId);
    q.addBindValue(zeroRow);
    bool ok = q.exec();
    if (!ok) logQuery(q);
    else DB_LOG("changePokeball: row=" << row << " delta=" << delta);
    return ok;
}

bool PokemonDatabase::addTechnicalMove(int moveId) {
    QSqlQuery q;
    q.prepare("INSERT OR IGNORE INTO technical_moves(save_id, move_id) VALUES(?, ?)");
    q.addBindValue(m_saveId);
    q.addBindValue(moveId);
    bool ok = q.exec();
    if (!ok) logQuery(q);
    else DB_LOG("addTechnicalMove: move_id=" << moveId);
    return ok;
}

bool PokemonDatabase::hasTechnicalMove(int moveId) {
    QSqlQuery q;
    q.prepare("SELECT 1 FROM technical_moves WHERE save_id=? AND move_id=?");
    q.addBindValue(m_saveId);
    q.addBindValue(moveId);
    bool found = q.exec() && q.next();
    if (!q.exec() && q.lastError().isValid()) logQuery(q);
    DB_LOG("hasTechnicalMove: move_id=" << moveId << " -> " << (found ? "yes" : "no"));
    return found;
}

std::vector<int> PokemonDatabase::getTechnicalMoveList() {
    QSqlQuery q;
    q.prepare("SELECT move_id FROM technical_moves WHERE save_id=?");
    q.addBindValue(m_saveId);
    std::vector<int> moves;
    if (q.exec()) {
        while (q.next()) {
            moves.push_back(q.value(0).toInt());
        }
    } else {
        logQuery(q);
    }
    DB_LOG("getTechnicalMoveList: found " << moves.size() << " moves");
    return moves;
}

std::vector<int> PokemonDatabase::filterKnownTMs(const std::vector<int>& moveIds) {
    if (moveIds.empty()) return {};

    QString placeholders;
    for (int i = 0; i < (int)moveIds.size(); ++i) {
        if (i > 0) placeholders += ',';
        placeholders += '?';
    }

    QSqlQuery q;
    q.prepare(QString("SELECT move_id FROM technical_moves WHERE save_id=? AND move_id IN (%1)").arg(placeholders));
    q.addBindValue(m_saveId);
    for (int id : moveIds) q.addBindValue(id);

    std::vector<int> result;
    if (q.exec()) {
        while (q.next()) result.push_back(q.value(0).toInt());
    } else {
        logQuery(q);
    }
    DB_LOG("filterKnownTMs: " << result.size() << "/" << moveIds.size() << " known");
    return result;
}

std::vector<EligibleEntry> PokemonDatabase::filterKnownTMs(const std::vector<EligibleEntry>& entries) {
    if (entries.empty()) return {};

    QString placeholders;
    for (int i = 0; i < (int)entries.size(); ++i) {
        if (i > 0) placeholders += ',';
        placeholders += '?';
    }

    QSqlQuery q;
    q.prepare(QString("SELECT move_id FROM technical_moves WHERE save_id=? AND move_id IN (%1)").arg(placeholders));
    q.addBindValue(m_saveId);
    for (const auto& e : entries) q.addBindValue(e.move_id);

    std::unordered_set<int> known;
    if (q.exec()) {
        while (q.next()) known.insert(q.value(0).toInt());
    } else {
        logQuery(q);
    }

    std::vector<EligibleEntry> result;
    for (const auto& e : entries)
        if (known.count(e.move_id)) result.push_back(e);
    return result;
}

bool PokemonDatabase::evolvePokemon(int boxIndex, int slot, int targetPokedexId, std::string name) {
    DB_LOG("evolvePokemon: box=" << boxIndex << " slot=" << slot << " target=" << targetPokedexId);
    PokemonState* p = cachePtr(boxIndex, slot);
    if (!p) {
        DB_WARN("evolvePokemon: invalid slot");
        return false;
    }
    if (p->empty()) {
        DB_WARN("evolvePokemon: slot empty");
        return false;
    }

    p->pokedex_id = targetPokedexId;
    p->name       = name;

    if (boxIndex == -1) {
        return dbWritePartySlot(slot, *p);
    } else {
        return dbWritePCSlot(boxIndex, slot, *p);
    }
}

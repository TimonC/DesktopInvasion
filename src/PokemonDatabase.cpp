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

#define DB_LOG(msg)  qDebug()    << "[DB]" << msg
#define DB_WARN(msg) qWarning()  << "[DB]" << msg
#define DB_ERR(msg)  qCritical() << "[DB]" << msg

static void logQuery(const QSqlQuery& q) {
    if (q.lastError().isValid())
        DB_ERR("Query failed:" << q.lastError().text() << "| SQL:" << q.lastQuery());
}

// --------------------------------------------------------------------------

PokemonDatabase& PokemonDatabase::instance() {
    static PokemonDatabase inst;
    return inst;
}

PokemonDatabase::~PokemonDatabase() { shutdown(); }

bool PokemonDatabase::initialize(const std::string& dbPath, int save_id) {
    if (m_initialized) return true;

    if (dbPath.empty()) {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        m_dbPath = (dir + "/pokemon.db").toStdString();
    } else {
        m_dbPath = dbPath;
    }

    QString qPath     = QString::fromStdString(m_dbPath);
    QDir    parentDir = QFileInfo(qPath).dir();
    if (!parentDir.exists()) parentDir.mkpath(".");
    if (!QFileInfo(parentDir.path()).isWritable()) {
        DB_ERR("DB dir not writable:" << parentDir.path());
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(qPath);
    if (!db.open()) {
        DB_ERR("Failed to open DB:" << db.lastError().text());
        return false;
    }

    DB_LOG("Opened at" << qPath);
    m_saveId      = save_id;
    m_initialized = true;

    createTables();
    initFixedSlots();
    loadWildAndParty();
    return true;
}

void PokemonDatabase::shutdown() {
    if (!m_initialized) return;
    DB_LOG("Shutdown");
    QSqlDatabase::database().close();
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    m_initialized = false;
}

// --------------------------------------------------------------------------
// Schema
// --------------------------------------------------------------------------

void PokemonDatabase::createTables() {
    QSqlQuery q;

    q.exec(R"(CREATE TABLE IF NOT EXISTS saves (
        save_id          INTEGER PRIMARY KEY,
        player_sprite_id INTEGER DEFAULT 0,
        name             TEXT    DEFAULT 'Player',
        current_box      INTEGER DEFAULT 0,
        unlocked_boxes   INTEGER DEFAULT 10
    ))"); logQuery(q);

    q.exec(R"(CREATE TABLE IF NOT EXISTS wild_slot (
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
        has_exp_share INTEGER DEFAULT 0,
        FOREIGN KEY(save_id) REFERENCES saves(save_id)
    ))"); logQuery(q);

    q.exec(R"(CREATE TABLE IF NOT EXISTS party_slots (
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
        has_exp_share INTEGER DEFAULT 0,
        PRIMARY KEY(save_id, slot),
        FOREIGN KEY(save_id) REFERENCES saves(save_id)
    ))"); logQuery(q);

    q.exec(R"(CREATE TABLE IF NOT EXISTS pc_slots (
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
        has_exp_share INTEGER DEFAULT 0,
        PRIMARY KEY(save_id, box, slot),
        FOREIGN KEY(save_id) REFERENCES saves(save_id)
    ))"); logQuery(q);

    q.exec("CREATE INDEX IF NOT EXISTS idx_pc_box ON pc_slots(save_id, box)");
    logQuery(q);
}

void PokemonDatabase::initFixedSlots() {
    QSqlQuery q;

    q.prepare("INSERT OR IGNORE INTO saves(save_id) VALUES(?)");
    q.addBindValue(m_saveId); q.exec(); logQuery(q);

    q.prepare("INSERT OR IGNORE INTO wild_slot(save_id) VALUES(?)");
    q.addBindValue(m_saveId); q.exec(); logQuery(q);

    q.prepare("INSERT OR IGNORE INTO party_slots(save_id, slot) VALUES(?, ?)");
    for (int i = 0; i < PARTY_SIZE; ++i) {
        q.addBindValue(m_saveId);
        q.addBindValue(i);
        q.exec(); logQuery(q);
    }
    DB_LOG("Fixed slots ready for save_id=" << m_saveId);
}

// --------------------------------------------------------------------------
// Row <-> PokemonState
// --------------------------------------------------------------------------

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
    p.hasExpShare = q.value("has_exp_share").toBool();
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
    q.addBindValue(p.moves[0]);
    q.addBindValue(p.moves[1]);
    q.addBindValue(p.moves[2]);
    q.addBindValue(p.moves[3]);
    q.addBindValue(p.hasExpShare ? 1 : 0);
}

// --------------------------------------------------------------------------
// Load caches
// --------------------------------------------------------------------------

void PokemonDatabase::loadWildAndParty() {
    QSqlQuery q;

    q.prepare("SELECT * FROM wild_slot WHERE save_id=?");
    q.addBindValue(m_saveId);
    if (q.exec() && q.next()) {
        m_wild = rowToPokemon(q);
        DB_LOG("Wild:" << (m_wild.empty() ? "empty" : QString::fromStdString(m_wild.name)));
    } else { logQuery(q); }

    q.prepare("SELECT * FROM party_slots WHERE save_id=? ORDER BY slot");
    q.addBindValue(m_saveId);
    if (q.exec()) {
        int count = 0;
        while (q.next()) {
            int slot      = q.value("slot").toInt();
            m_party[slot] = rowToPokemon(q);
            if (!m_party[slot].empty()) ++count;
        }
        DB_LOG("Party:" << count << "/ 6 slots occupied");
    } else { logQuery(q); }
}

// --------------------------------------------------------------------------
// DB write helpers
// --------------------------------------------------------------------------

void PokemonDatabase::dbWriteWild(const PokemonState& p) {
    QSqlQuery q;
    q.prepare(R"(UPDATE wild_slot SET
        pokedex_id=?, variant_id=?, pokeball_id=?, name=?,
        lvl=?, current_xp=?, nature=?,
        move0=?, move1=?, move2=?, move3=?, has_exp_share=?
        WHERE save_id=?)");
    writePokemonToRow(q, p);
    q.addBindValue(m_saveId);
    q.exec(); logQuery(q);
    DB_LOG("Wild ->" << (p.empty() ? "empty" : QString::fromStdString(p.name)));
}

void PokemonDatabase::dbWritePartySlot(int slot, const PokemonState& p) {
    QSqlQuery q;
    q.prepare(R"(UPDATE party_slots SET
        pokedex_id=?, variant_id=?, pokeball_id=?, name=?,
        lvl=?, current_xp=?, nature=?,
        move0=?, move1=?, move2=?, move3=?, has_exp_share=?
        WHERE save_id=? AND slot=?)");
    writePokemonToRow(q, p);
    q.addBindValue(m_saveId);
    q.addBindValue(slot);
    q.exec(); logQuery(q);
    DB_LOG("Party[" << slot << "] ->" << (p.empty() ? "empty" : QString::fromStdString(p.name)));
}

void PokemonDatabase::dbWritePCSlot(int box, int slot, const PokemonState& p) {
    if (p.empty()) { dbDeletePCSlot(box, slot); return; }
    QSqlQuery q;
    q.prepare(R"(INSERT OR REPLACE INTO pc_slots
        (save_id, box, slot, pokedex_id, variant_id, pokeball_id, name,
         lvl, current_xp, nature, move0, move1, move2, move3, has_exp_share)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?))");
    q.addBindValue(m_saveId);
    q.addBindValue(box);
    q.addBindValue(slot);
    writePokemonToRow(q, p);
    q.exec(); logQuery(q);
    DB_LOG("PC[" << box << "][" << slot << "] ->" << QString::fromStdString(p.name));
}

void PokemonDatabase::dbDeletePCSlot(int box, int slot) {
    QSqlQuery q;
    q.prepare("DELETE FROM pc_slots WHERE save_id=? AND box=? AND slot=?");
    q.addBindValue(m_saveId);
    q.addBindValue(box);
    q.addBindValue(slot);
    q.exec(); logQuery(q);
    DB_LOG("PC[" << box << "][" << slot << "] -> cleared");
}

// --------------------------------------------------------------------------
// Wild
// --------------------------------------------------------------------------

void PokemonDatabase::setWild(const PokemonState& p) {
    m_wild = p;
    dbWriteWild(p);
}

void PokemonDatabase::clearWild() {
    m_wild = PokemonState{};
    dbWriteWild(m_wild);
}

// --------------------------------------------------------------------------
// Party
// --------------------------------------------------------------------------

void PokemonDatabase::setPartySlot(int slot, const PokemonState& p) {
    assert(slot >= 0 && slot < PARTY_SIZE);
    m_party[slot] = p;
    dbWritePartySlot(slot, p);
}

int PokemonDatabase::firstFreePartySlot() const {
    for (int i = 0; i < PARTY_SIZE; ++i)
        if (m_party[i].empty()) return i;
    return -1;
}

int PokemonDatabase::partySize() const {
    int n = 0;
    for (const auto& p : m_party) if (!p.empty()) ++n;
    return n;
}

// --------------------------------------------------------------------------
// PC
// --------------------------------------------------------------------------

void PokemonDatabase::loadBox(int box) {
    if (m_boxCache.count(box)) return;

    std::array<PokemonState, BOX_SIZE> arr{};
    QSqlQuery q;
    q.prepare("SELECT * FROM pc_slots WHERE save_id=? AND box=?");
    q.addBindValue(m_saveId);
    q.addBindValue(box);
    if (q.exec()) {
        while (q.next())
            arr[q.value("slot").toInt()] = rowToPokemon(q);
    } else { logQuery(q); }

    m_boxCache[box] = arr;
    DB_LOG("PC box" << box << "loaded");
}

bool PokemonDatabase::isBoxLoaded(int box) const {
    return m_boxCache.count(box) > 0;
}

const std::array<PokemonState, BOX_SIZE>& PokemonDatabase::getBox(int box) const {
    assert(isBoxLoaded(box));
    return m_boxCache.at(box);
}

void PokemonDatabase::setPCSlot(int box, int slot, const PokemonState& p) {
    if (!isBoxLoaded(box)) loadBox(box);
    m_boxCache[box][slot] = p;
    dbWritePCSlot(box, slot, p);
}

void PokemonDatabase::swapPCSlots(int boxA, int slotA, int boxB, int slotB) {
    if (!isBoxLoaded(boxA)) loadBox(boxA);
    if (!isBoxLoaded(boxB)) loadBox(boxB);
    std::swap(m_boxCache[boxA][slotA], m_boxCache[boxB][slotB]);
    dbWritePCSlot(boxA, slotA, m_boxCache[boxA][slotA]);
    dbWritePCSlot(boxB, slotB, m_boxCache[boxB][slotB]);
    DB_LOG("PC swapped [" << boxA << "][" << slotA << "] <-> [" << boxB << "][" << slotB << "]");
}

// --------------------------------------------------------------------------
// Catch
// --------------------------------------------------------------------------

std::pair<int, int> PokemonDatabase::firstFreePC() {
    for (int box = 0; box < MAX_BOXES; ++box) {
        loadBox(box);
        for (int slot = 0; slot < BOX_SIZE; ++slot)
            if (m_boxCache[box][slot].empty()) return {box, slot};
    }
    return {-1, -1};
}

std::pair<int, int> PokemonDatabase::catchWildPokemon(int pokeball_id) {
    if (m_wild.empty()) return {-1, -1};

    PokemonState caught = m_wild;
    caught.pokeball_id  = pokeball_id;

    int partySlot = firstFreePartySlot();
    if (partySlot >= 0) {
        setPartySlot(partySlot, caught);
        clearWild();
        DB_LOG("Caught" << QString::fromStdString(caught.name) << "-> party[" << partySlot << "]");
        return {-1, partySlot};
    }

    auto [box, slot] = firstFreePC();
    if (box < 0) {
        DB_WARN("Storage full! Could not catch" << QString::fromStdString(caught.name));
        return {-1, -1};
    }

    setPCSlot(box, slot, caught);
    clearWild();
    DB_LOG("Caught" << QString::fromStdString(caught.name) << "-> PC[" << box << "][" << slot << "]");
    return {box, slot};
}

// --------------------------------------------------------------------------
// Game state
// --------------------------------------------------------------------------

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
    } else {
        saveGameState(state);
    }
    return state;
}

bool PokemonDatabase::saveGameState(const GameState& state) {
    QSqlQuery q;
    q.prepare(R"(INSERT OR REPLACE INTO saves
        (save_id, player_sprite_id, name, current_box, unlocked_boxes)
        VALUES(?,?,?,?,?))");
    q.addBindValue(state.save_id);
    q.addBindValue(state.player_sprite_id);
    q.addBindValue(QString::fromStdString(state.name));
    q.addBindValue(state.current_box);
    q.addBindValue(state.unlocked_boxes);
    q.exec(); logQuery(q);
    DB_LOG("GameState saved (save_id=" << state.save_id << ")");
    return !q.lastError().isValid();
}

// --------------------------------------------------------------------------
// Menu session — one open transaction, writes happen immediately to cache+DB,
// commit flushes, rollback reverts both DB and cache.
// --------------------------------------------------------------------------

void PokemonDatabase::beginMenuSession() {
    assert(!m_inMenuSession);
    QSqlDatabase::database().transaction();
    m_inMenuSession = true;
    DB_LOG("Menu session started");
}

void PokemonDatabase::swapByPos(int boxX, int slotX, int boxY, int slotY) {
    assert(m_inMenuSession);
    if (boxX == boxY && slotX == slotY) return;

    const bool xIsParty = (boxX == -1);
    const bool yIsParty = (boxY == -1);

    if (xIsParty && yIsParty) {
        std::swap(m_party[slotX], m_party[slotY]);
        dbWritePartySlot(slotX, m_party[slotX]);
        dbWritePartySlot(slotY, m_party[slotY]);
    } else if (!xIsParty && !yIsParty) {
        swapPCSlots(boxX, slotX, boxY, slotY);
    } else {
        const int pcBox   = xIsParty ? boxY  : boxX;
        const int pcSlot  = xIsParty ? slotY : slotX;
        const int pSlot   = xIsParty ? slotX : slotY;
        if (!isBoxLoaded(pcBox)) loadBox(pcBox);
        std::swap(m_boxCache[pcBox][pcSlot], m_party[pSlot]);
        dbWritePCSlot(pcBox, pcSlot, m_boxCache[pcBox][pcSlot]);
        dbWritePartySlot(pSlot, m_party[pSlot]);
    }
}

void PokemonDatabase::commitMenuSession() {
    assert(m_inMenuSession);
    QSqlDatabase::database().commit();
    m_inMenuSession = false;
    DB_LOG("Menu session committed");
}

void PokemonDatabase::rollbackMenuSession() {
    assert(m_inMenuSession);
    QSqlDatabase::database().rollback();
    // Reload affected caches from DB to stay consistent
    m_boxCache.clear();
    loadWildAndParty();
    m_inMenuSession = false;
    DB_LOG("Menu session rolled back");
}

// --------------------------------------------------------------------------
// Exp share
// --------------------------------------------------------------------------

bool PokemonDatabase::toggleExpShare(int partySlot) {
    assert(partySlot >= 0 && partySlot < PARTY_SIZE);
    m_party[partySlot].hasExpShare = !m_party[partySlot].hasExpShare;
    dbWritePartySlot(partySlot, m_party[partySlot]);
    DB_LOG("ExpShare slot" << partySlot << "->" << m_party[partySlot].hasExpShare);
    return m_party[partySlot].hasExpShare;
}

std::vector<int> PokemonDatabase::partyExpShareSlots() {
    std::vector<int> result;
    for (int i = 0; i < PARTY_SIZE; ++i)
        if (!m_party[i].empty() && m_party[i].hasExpShare)
            result.push_back(i);
    return result;
}

// --------------------------------------------------------------------------
// Multi-save
// --------------------------------------------------------------------------

std::vector<GameState> PokemonDatabase::listSaves() {
    std::vector<GameState> saves;
    QSqlQuery q("SELECT * FROM saves");
    while (q.next()) {
        GameState s;
        s.save_id          = q.value("save_id").toInt();
        s.player_sprite_id = q.value("player_sprite_id").toInt();
        s.name             = q.value("name").toString().toStdString();
        s.current_box      = q.value("current_box").toInt();
        s.unlocked_boxes   = q.value("unlocked_boxes").toInt();
        saves.push_back(s);
    }
    return saves;
}

bool PokemonDatabase::switchSave(int save_id) {
    DB_LOG("Switching to save_id=" << save_id);
    m_saveId        = save_id;
    m_boxCache.clear();
    m_inMenuSession = false;
    initFixedSlots();
    loadWildAndParty();
    return true;
}
// --------------------------------------------------------------------------
// Field patches
// --------------------------------------------------------------------------

// Internal helper: run a targeted UPDATE on whichever table owns this slot.
// box == -2 → wild_slot   (slot ignored)
// box == -1 → party_slots (slot = party index)
// box >= 0  → pc_slots    (box + slot)
//
// 'setCols' is the SET fragment, e.g. "name=?"
// Bind your value(s) BEFORE calling; the WHERE params are appended here.
static void patchSlot(int saveId, int box, int slot,
                      const QString& setCols,
                      std::function<void(QSqlQuery&)> bindValues)
{
    QSqlQuery q;
    if (box == -2) {
        q.prepare(QString("UPDATE wild_slot SET %1 WHERE save_id=?").arg(setCols));
        bindValues(q);
        q.addBindValue(saveId);
    } else if (box == -1) {
        q.prepare(QString("UPDATE party_slots SET %1 WHERE save_id=? AND slot=?").arg(setCols));
        bindValues(q);
        q.addBindValue(saveId);
        q.addBindValue(slot);
    } else {
        q.prepare(QString("UPDATE pc_slots SET %1 WHERE save_id=? AND box=? AND slot=?").arg(setCols));
        bindValues(q);
        q.addBindValue(saveId);
        q.addBindValue(box);
        q.addBindValue(slot);
    }
    q.exec();
    logQuery(q);
}

// Patch cache helper: returns a pointer to the live PokemonState, or nullptr.
PokemonState* PokemonDatabase::cachePtr(int box, int slot) {
    if (box == -2) return &m_wild;
    if (box == -1) { return (slot >= 0 && slot < PARTY_SIZE) ? &m_party[slot] : nullptr; }
    if (!isBoxLoaded(box)) loadBox(box);
    return (slot >= 0 && slot < BOX_SIZE) ? &m_boxCache[box][slot] : nullptr;
}

void PokemonDatabase::renamePokemon(int box, int slot, const std::string& newName) {
    PokemonState* p = cachePtr(box, slot);
    if (!p || p->empty()) return;

    p->name = newName;
    QString qName = QString::fromStdString(newName);
    patchSlot(m_saveId, box, slot, "name=?",
              [&](QSqlQuery& q){ q.addBindValue(qName); });

    DB_LOG("Renamed [box=" << box << " slot=" << slot << "] ->" << qName);
}

void PokemonDatabase::setPokemonMoves(int box, int slot, const int moves[4]) {
    PokemonState* p = cachePtr(box, slot);
    if (!p || p->empty()) return;

    for (int i = 0; i < 4; ++i) p->moves[i] = moves[i];
    patchSlot(m_saveId, box, slot, "move0=?, move1=?, move2=?, move3=?",
              [&](QSqlQuery& q){
                  q.addBindValue(moves[0]);
                  q.addBindValue(moves[1]);
                  q.addBindValue(moves[2]);
                  q.addBindValue(moves[3]);
              });

    DB_LOG("Moves [box=" << box << " slot=" << slot << "] ->"
           << moves[0] << moves[1] << moves[2] << moves[3]);
}

void PokemonDatabase::setPokemonMove(int box, int slot, int moveIndex, int moveId) {
    if (moveIndex < 0 || moveIndex > 3) return;
    PokemonState* p = cachePtr(box, slot);
    if (!p || p->empty()) return;

    p->moves[moveIndex] = moveId;
    static const char* cols[] = {"move0=?","move1=?","move2=?","move3=?"};
    patchSlot(m_saveId, box, slot, cols[moveIndex],
              [&](QSqlQuery& q){ q.addBindValue(moveId); });

    DB_LOG("Move[" << moveIndex << "] [box=" << box << " slot=" << slot << "] ->" << moveId);
}

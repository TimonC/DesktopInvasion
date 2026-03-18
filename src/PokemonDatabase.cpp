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

    QString qPath = QString::fromStdString(m_dbPath);
    QDir parentDir = QFileInfo(qPath).dir();
    if (!parentDir.exists()) parentDir.mkpath(".");
    if (!QFileInfo(parentDir.path()).isWritable()) {
        qCritical() << "DB dir not writable:" << parentDir.path();
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(qPath);
    if (!db.open()) {
        qCritical() << "Failed to open DB:" << db.lastError().text();
        return false;
    }

    m_saveId      = save_id;
    m_initialized = true;

    createTables();
    initFixedSlots();
    loadWildAndParty();

    return true;
}

void PokemonDatabase::shutdown() {
    if (!m_initialized) return;
    QSqlDatabase::database().close();
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    m_initialized = false;
}

// --------------------------------------------------------------------------
// Schema
// --------------------------------------------------------------------------

void PokemonDatabase::createTables() {
    QSqlQuery q;

    // Saves table
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS saves (
            save_id          INTEGER PRIMARY KEY,
            player_sprite_id INTEGER DEFAULT 0,
            name             TEXT    DEFAULT 'Player',
            current_box      INTEGER DEFAULT 0,
            unlocked_boxes   INTEGER DEFAULT 10
        )
    )");

    // Wild: 1 row per save
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS wild_slot (
            save_id     INTEGER PRIMARY KEY,
            pokedex_id  INTEGER DEFAULT 0,
            variant_id  INTEGER DEFAULT 0,
            pokeball_id INTEGER DEFAULT 0,
            name        TEXT    DEFAULT '',
            lvl         INTEGER DEFAULT 1,
            current_xp  INTEGER DEFAULT 0,
            nature      INTEGER DEFAULT 0,
            move0       INTEGER DEFAULT 0,
            move1       INTEGER DEFAULT 0,
            move2       INTEGER DEFAULT 0,
            move3       INTEGER DEFAULT 0,
            has_exp_share INTEGER DEFAULT 0,
            FOREIGN KEY(save_id) REFERENCES saves(save_id)
        )
    )");

    // Party: exactly PARTY_SIZE rows per save
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS party_slots (
            save_id     INTEGER NOT NULL,
            slot        INTEGER NOT NULL,
            pokedex_id  INTEGER DEFAULT 0,
            variant_id  INTEGER DEFAULT 0,
            pokeball_id INTEGER DEFAULT 0,
            name        TEXT    DEFAULT '',
            lvl         INTEGER DEFAULT 1,
            current_xp  INTEGER DEFAULT 0,
            nature      INTEGER DEFAULT 0,
            move0       INTEGER DEFAULT 0,
            move1       INTEGER DEFAULT 0,
            move2       INTEGER DEFAULT 0,
            move3       INTEGER DEFAULT 0,
            has_exp_share INTEGER DEFAULT 0,
            PRIMARY KEY(save_id, slot),
            FOREIGN KEY(save_id) REFERENCES saves(save_id)
        )
    )");

    // PC: sparse, one row per occupied slot
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS pc_slots (
            save_id     INTEGER NOT NULL,
            box         INTEGER NOT NULL,
            slot        INTEGER NOT NULL,
            pokedex_id  INTEGER DEFAULT 0,
            variant_id  INTEGER DEFAULT 0,
            pokeball_id INTEGER DEFAULT 0,
            name        TEXT    DEFAULT '',
            lvl         INTEGER DEFAULT 1,
            current_xp  INTEGER DEFAULT 0,
            nature      INTEGER DEFAULT 0,
            move0       INTEGER DEFAULT 0,
            move1       INTEGER DEFAULT 0,
            move2       INTEGER DEFAULT 0,
            move3       INTEGER DEFAULT 0,
            has_exp_share INTEGER DEFAULT 0,
            PRIMARY KEY(save_id, box, slot),
            FOREIGN KEY(save_id) REFERENCES saves(save_id)
        )
    )");

    q.exec("CREATE INDEX IF NOT EXISTS idx_pc_box ON pc_slots(save_id, box)");
}

void PokemonDatabase::initFixedSlots() {
    QSqlQuery q;

    // Ensure save row
    q.prepare("INSERT OR IGNORE INTO saves(save_id) VALUES(?)");
    q.addBindValue(m_saveId);
    q.exec();

    // Ensure wild row
    q.prepare("INSERT OR IGNORE INTO wild_slot(save_id) VALUES(?)");
    q.addBindValue(m_saveId);
    q.exec();

    // Ensure all party rows
    q.prepare("INSERT OR IGNORE INTO party_slots(save_id, slot) VALUES(?, ?)");
    for (int i = 0; i < PARTY_SIZE; ++i) {
        q.addBindValue(m_saveId);
        q.addBindValue(i);
        q.exec();
    }
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

// Binds all pokemon fields (no save_id / slot / box — caller adds those first)
void PokemonDatabase::writePokemonToRow(QSqlQuery& q, const PokemonState& p, bool bindSlotParams) {
    (void)bindSlotParams;
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
    if (q.exec() && q.next())
        m_wild = rowToPokemon(q);

    q.prepare("SELECT * FROM party_slots WHERE save_id=? ORDER BY slot");
    q.addBindValue(m_saveId);
    if (q.exec()) {
        while (q.next()) {
            int slot = q.value("slot").toInt();
            m_party[slot] = rowToPokemon(q);
        }
    }
}

// --------------------------------------------------------------------------
// DB write helpers
// --------------------------------------------------------------------------

void PokemonDatabase::dbWriteWild(const PokemonState& p) {
    QSqlQuery q;
    q.prepare(R"(
        UPDATE wild_slot SET
            pokedex_id=?, variant_id=?, pokeball_id=?, name=?,
            lvl=?, current_xp=?, nature=?,
            move0=?, move1=?, move2=?, move3=?, has_exp_share=?
        WHERE save_id=?
    )");
    writePokemonToRow(q, p, false);
    q.addBindValue(m_saveId);
    q.exec();
}

void PokemonDatabase::dbWritePartySlot(int slot, const PokemonState& p) {
    QSqlQuery q;
    q.prepare(R"(
        UPDATE party_slots SET
            pokedex_id=?, variant_id=?, pokeball_id=?, name=?,
            lvl=?, current_xp=?, nature=?,
            move0=?, move1=?, move2=?, move3=?, has_exp_share=?
        WHERE save_id=? AND slot=?
    )");
    writePokemonToRow(q, p, false);
    q.addBindValue(m_saveId);
    q.addBindValue(slot);
    q.exec();
}

void PokemonDatabase::dbWritePCSlot(int box, int slot, const PokemonState& p) {
    QSqlQuery q;
    if (p.empty()) {
        dbDeletePCSlot(box, slot);
        return;
    }
    q.prepare(R"(
        INSERT OR REPLACE INTO pc_slots
            (save_id, box, slot, pokedex_id, variant_id, pokeball_id, name,
             lvl, current_xp, nature, move0, move1, move2, move3, has_exp_share)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
    )");
    q.addBindValue(m_saveId);
    q.addBindValue(box);
    q.addBindValue(slot);
    writePokemonToRow(q, p, false);
    q.exec();
}

void PokemonDatabase::dbDeletePCSlot(int box, int slot) {
    QSqlQuery q;
    q.prepare("DELETE FROM pc_slots WHERE save_id=? AND box=? AND slot=?");
    q.addBindValue(m_saveId);
    q.addBindValue(box);
    q.addBindValue(slot);
    q.exec();
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

void PokemonDatabase::clearPartySlot(int slot) {
    assert(slot >= 0 && slot < PARTY_SIZE);
    m_party[slot] = PokemonState{};
    dbWritePartySlot(slot, m_party[slot]);
}

void PokemonDatabase::swapPartySlots(int a, int b) {
    assert(a >= 0 && a < PARTY_SIZE && b >= 0 && b < PARTY_SIZE);
    std::swap(m_party[a], m_party[b]);
    QSqlDatabase::database().transaction();
    dbWritePartySlot(a, m_party[a]);
    dbWritePartySlot(b, m_party[b]);
    QSqlDatabase::database().commit();
}

// --------------------------------------------------------------------------
// PC
// --------------------------------------------------------------------------

void PokemonDatabase::loadBox(int box) {
    if (m_boxCache.count(box)) return;

    std::array<PokemonState, BOX_SIZE> arr;

    QSqlQuery q;
    q.prepare("SELECT * FROM pc_slots WHERE save_id=? AND box=?");
    q.addBindValue(m_saveId);
    q.addBindValue(box);
    if (q.exec()) {
        while (q.next()) {
            int slot    = q.value("slot").toInt();
            arr[slot]   = rowToPokemon(q);
        }
    }
    m_boxCache[box] = arr;
}

void PokemonDatabase::prefetchBox(int box) {
    if (box >= 0 && box < MAX_BOXES)
        loadBox(box); // synchronous for now; can be made async later
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

    QSqlDatabase::database().transaction();
    dbWritePCSlot(boxA, slotA, m_boxCache[boxA][slotA]);
    dbWritePCSlot(boxB, slotB, m_boxCache[boxB][slotB]);
    QSqlDatabase::database().commit();
}

// --------------------------------------------------------------------------
// Catch
// --------------------------------------------------------------------------

std::pair<int,int> PokemonDatabase::firstFreePC() {
    for (int box = 0; box < MAX_BOXES; ++box) {
        loadBox(box);
        for (int slot = 0; slot < BOX_SIZE; ++slot) {
            if (m_boxCache[box][slot].empty())
                return {box, slot};
        }
    }
    return {-1, -1};
}

std::pair<int,int> PokemonDatabase::catchWildPokemon(int pokeball_id) {
    if (m_wild.empty()) return {-1, -1};

    PokemonState caught  = m_wild;
    caught.pokeball_id   = pokeball_id;

    auto [box, slot]     = firstFreePC();
    if (box < 0) return {-1, -1};

    setPCSlot(box, slot, caught);
    clearWild();
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
    q.prepare(R"(
        INSERT OR REPLACE INTO saves(save_id, player_sprite_id, name, current_box, unlocked_boxes)
        VALUES(?,?,?,?,?)
    )");
    q.addBindValue(state.save_id);
    q.addBindValue(state.player_sprite_id);
    q.addBindValue(QString::fromStdString(state.name));
    q.addBindValue(state.current_box);
    q.addBindValue(state.unlocked_boxes);
    return q.exec();
}

// --------------------------------------------------------------------------
// Menu session
// --------------------------------------------------------------------------

void PokemonDatabase::beginMenuSession() {
    assert(!m_inMenuSession);
    m_wildSnapshot  = m_wild;
    m_partySnapshot = m_party;
    m_pendingChanges.clear();
    m_inMenuSession = true;
}

void PokemonDatabase::queuePartyChange(int slot, const PokemonState& p) {
    m_pendingChanges.push_back({PendingSlotChange::PartySet, -1, slot, -1, -1, p});
    m_party[slot] = p; // update live cache for UI
}

void PokemonDatabase::queuePartySwap(int slotA, int slotB) {
    m_pendingChanges.push_back({PendingSlotChange::PartySwap, -1, slotA, -1, slotB, {}});
    std::swap(m_party[slotA], m_party[slotB]);
}

void PokemonDatabase::queuePCChange(int box, int slot, const PokemonState& p) {
    if (!isBoxLoaded(box)) loadBox(box);
    m_pendingChanges.push_back({PendingSlotChange::PCSet, box, slot, -1, -1, p});
    m_boxCache[box][slot] = p;
}

void PokemonDatabase::queuePCSwap(int boxA, int slotA, int boxB, int slotB) {
    if (!isBoxLoaded(boxA)) loadBox(boxA);
    if (!isBoxLoaded(boxB)) loadBox(boxB);
    m_pendingChanges.push_back({PendingSlotChange::PCSwap, boxA, slotA, boxB, slotB, {}});
    std::swap(m_boxCache[boxA][slotA], m_boxCache[boxB][slotB]);
}

void PokemonDatabase::queuePCToParty(int box, int slot, int partySlot) {
    if (!isBoxLoaded(box)) loadBox(box);
    PokemonState moved  = m_boxCache[box][slot];
    PokemonState evicted= m_party[partySlot];

    m_pendingChanges.push_back({PendingSlotChange::PCToParty, box, slot, -1, partySlot, {}});
    m_party[partySlot]     = moved;
    m_boxCache[box][slot]  = evicted;
}

void PokemonDatabase::queuePartyToPC(int partySlot, int box, int slot) {
    if (!isBoxLoaded(box)) loadBox(box);
    PokemonState moved  = m_party[partySlot];
    PokemonState evicted= m_boxCache[box][slot];

    m_pendingChanges.push_back({PendingSlotChange::PartyToPC, box, slot, -1, partySlot, {}});
    m_boxCache[box][slot]  = moved;
    m_party[partySlot]     = evicted;
}

void PokemonDatabase::commitMenuSession() {
    assert(m_inMenuSession);

    QSqlDatabase::database().transaction();

    for (const auto& change : m_pendingChanges) {
        switch (change.kind) {
        case PendingSlotChange::PartySet:
            dbWritePartySlot(change.a_slot, *change.data);
            break;
        case PendingSlotChange::PartySwap:
            dbWritePartySlot(change.a_slot, m_party[change.a_slot]);
            dbWritePartySlot(change.b_slot, m_party[change.b_slot]);
            break;
        case PendingSlotChange::PCSet:
            dbWritePCSlot(change.a_box, change.a_slot, *change.data);
            break;
        case PendingSlotChange::PCSwap:
            dbWritePCSlot(change.a_box, change.a_slot, m_boxCache[change.a_box][change.a_slot]);
            dbWritePCSlot(change.b_box, change.b_slot, m_boxCache[change.b_box][change.b_slot]);
            break;
        case PendingSlotChange::PCToParty:
            dbWritePartySlot(change.b_slot, m_party[change.b_slot]);
            dbWritePCSlot(change.a_box, change.a_slot, m_boxCache[change.a_box][change.a_slot]);
            break;
        case PendingSlotChange::PartyToPC:
            dbWritePartySlot(change.b_slot, m_party[change.b_slot]);
            dbWritePCSlot(change.a_box, change.a_slot, m_boxCache[change.a_box][change.a_slot]);
            break;
        }
    }

    QSqlDatabase::database().commit();
    m_pendingChanges.clear();
    m_inMenuSession = false;
}

void PokemonDatabase::rollbackMenuSession() {
    assert(m_inMenuSession);
    m_wild  = m_wildSnapshot;
    m_party = m_partySnapshot;
    // PC cache: evict any boxes that were touched so they'll reload cleanly
    for (const auto& change : m_pendingChanges) {
        if (change.a_box >= 0) m_boxCache.erase(change.a_box);
        if (change.b_box >= 0) m_boxCache.erase(change.b_box);
    }
    m_pendingChanges.clear();
    m_inMenuSession = false;
}

// --------------------------------------------------------------------------
// Exp share
// --------------------------------------------------------------------------

bool PokemonDatabase::toggleExpShare(int partySlot) {
    assert(partySlot >= 0 && partySlot < PARTY_SIZE);
    m_party[partySlot].hasExpShare = !m_party[partySlot].hasExpShare;
    dbWritePartySlot(partySlot, m_party[partySlot]);
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
    m_saveId = save_id;
    m_boxCache.clear();
    m_pendingChanges.clear();
    m_inMenuSession = false;
    initFixedSlots();
    loadWildAndParty();
    return true;
}

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

static bool execQuery(QSqlQuery& q, const QString& sql) {
    if (!q.prepare(sql) || !q.exec()) { logQuery(q); return false; }
    return true;
}

PokemonDatabase& PokemonDatabase::instance() {
    static PokemonDatabase inst;
    return inst;
}

PokemonDatabase::~PokemonDatabase() { shutdown(); }

bool PokemonDatabase::initialize(const std::string& dbPath, int save_id) {
    if (m_initialized) return true;

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        DB_ERR("SQLITE driver not available");
        return false;
    }

    QString path;
    if (dbPath.empty()) {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (!QDir().mkpath(dir)) {
            DB_ERR("Failed to create app data directory:" << dir);
            return false;
        }
        path = dir + "/pokemon.db";
    } else {
        path = QString::fromStdString(dbPath);
    }
    m_dbPath = path;

    QDir parentDir = QFileInfo(path).dir();
    if (!parentDir.exists() && !parentDir.mkpath(".")) {
        DB_ERR("Failed to create database directory:" << parentDir.path());
        return false;
    }
    if (!QFileInfo(parentDir.path()).isWritable()) {
        DB_ERR("DB dir not writable:" << parentDir.path());
        return false;
    }

    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
        DB_ERR("Failed to open DB:" << db.lastError().text());
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
        return false;
    }

    DB_LOG("Opened at" << path);
    m_saveId = save_id;
    m_initialized = true;

    if (!createTables())    { DB_ERR("Failed to create tables");          shutdown(); return false; }
    if (!initFixedSlots())  { DB_ERR("Failed to initialize fixed slots"); shutdown(); return false; }
    if (!loadWildAndParty()) { DB_ERR("Failed to load wild and party");    shutdown(); return false; }
    return true;
}

void PokemonDatabase::shutdown() {
    if (!m_initialized) return;
    DB_LOG("Shutdown");

    {
        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            db.close();
        }
    }

    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }

    m_initialized = false;
}

bool PokemonDatabase::createTables() {
    QSqlQuery q;
    bool ok = true;
    auto run = [&](const char* sql) { if (!q.exec(sql)) { logQuery(q); ok = false; } };

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
        scale          INTEGER DEFAULT 1,
        speed          INTEGER DEFAULT 1,
        lvl_range_up   INTEGER DEFAULT 5,
        lvl_range_down INTEGER DEFAULT 5,
        exp_share_on   INTEGER DEFAULT 0,
        FOREIGN KEY(save_id) REFERENCES saves(save_id)
    ))");
    run("CREATE INDEX IF NOT EXISTS idx_pc_box ON pc_slots(save_id, box)");

    return ok;
}

bool PokemonDatabase::initFixedSlots() {
    QSqlQuery q;
    bool ok = true;
    auto run = [&](const QString& sql, auto... vals) {
        q.prepare(sql);
        (q.addBindValue(vals), ...);
        if (!q.exec()) { logQuery(q); ok = false; }
    };

    run("INSERT OR IGNORE INTO saves(save_id) VALUES(?)", m_saveId);
    run("INSERT OR IGNORE INTO wild_slot(save_id) VALUES(?)", m_saveId);
    run("INSERT OR IGNORE INTO defaults(save_id) VALUES(?)", m_saveId);

    q.prepare("INSERT OR IGNORE INTO party_slots(save_id, slot) VALUES(?, ?)");
    for (int i = 0; i < PARTY_SIZE; ++i) {
        q.addBindValue(m_saveId);
        q.addBindValue(i);
        if (!q.exec()) { logQuery(q); ok = false; }
    }

    if (ok) DB_LOG("Fixed slots ready for save_id=" << m_saveId);
    return ok;
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
    q.addBindValue(p.moves[0]);
    q.addBindValue(p.moves[1]);
    q.addBindValue(p.moves[2]);
    q.addBindValue(p.moves[3]);
}

bool PokemonDatabase::loadWildAndParty() {
    QSqlQuery q;
    bool ok = true;

    q.prepare("SELECT * FROM wild_slot WHERE save_id=?");
    q.addBindValue(m_saveId);
    if (q.exec()) {
        if (q.next()) {
            m_wild = rowToPokemon(q);
            DB_LOG("Wild:" << (m_wild.empty() ? "empty" : QString::fromStdString(m_wild.name)));
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
        DB_LOG("Party:" << count << "/ 6 slots occupied");
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
    else DB_LOG("Wild ->" << (p.empty() ? "empty" : QString::fromStdString(p.name)));
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
    else DB_LOG("Party[" << slot << "] ->" << (p.empty() ? "empty" : QString::fromStdString(p.name)));
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
    else DB_LOG("PC[" << box << "][" << slot << "] ->" << QString::fromStdString(p.name));
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
    m_wild = p;
    dbWriteWild(p);
}

void PokemonDatabase::clearWild() {
    m_wild = PokemonState{};
    dbWriteWild(m_wild);
}

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

bool PokemonDatabase::loadBox(int box) {
    if (m_boxCache.count(box)) return true;

    QSqlQuery q;
    q.prepare("SELECT * FROM pc_slots WHERE save_id=? AND box=?");
    q.addBindValue(m_saveId);
    q.addBindValue(box);
    if (!q.exec()) { logQuery(q); return false; }

    std::array<PokemonState, BOX_SIZE> arr{};
    while (q.next())
        arr[q.value("slot").toInt()] = rowToPokemon(q);
    m_boxCache[box] = arr;
    DB_LOG("PC box" << box << "loaded");
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
    if (!isBoxLoaded(box) && !loadBox(box)) return false;
    m_boxCache[box][slot] = p;
    return dbWritePCSlot(box, slot, p);
}

bool PokemonDatabase::swapPCSlots(int boxA, int slotA, int boxB, int slotB) {
    if (!isBoxLoaded(boxA) && !loadBox(boxA)) return false;
    if (!isBoxLoaded(boxB) && !loadBox(boxB)) return false;
    std::swap(m_boxCache[boxA][slotA], m_boxCache[boxB][slotB]);
    bool ok1 = dbWritePCSlot(boxA, slotA, m_boxCache[boxA][slotA]);
    bool ok2 = dbWritePCSlot(boxB, slotB, m_boxCache[boxB][slotB]);
    if (ok1 && ok2) DB_LOG("PC swapped [" << boxA << "][" << slotA << "] <-> [" << boxB << "][" << slotB << "]");
    return ok1 && ok2;
}

std::pair<int, int> PokemonDatabase::firstFreePC() {
    for (int box = 0; box < MAX_BOXES; ++box) {
        if (!loadBox(box)) { DB_WARN("firstFreePC: failed to load box" << box); continue; }
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
    if (!setPCSlot(box, slot, caught)) {
        DB_ERR("Failed to write caught Pokemon to PC");
        return {-1, -1};
    }
    clearWild();
    DB_LOG("Caught" << QString::fromStdString(caught.name) << "-> PC[" << box << "][" << slot << "]");
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
    } else {
        logQuery(q);
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
    bool ok = q.exec();
    if (!ok) logQuery(q);
    else DB_LOG("GameState saved (save_id=" << state.save_id << ")");
    return ok;
}

bool PokemonDatabase::beginMenuSession() {
    assert(!m_inMenuSession);
    if (!QSqlDatabase::database().transaction()) {
        DB_ERR("Failed to begin transaction");
        return false;
    }
    m_inMenuSession = true;
    DB_LOG("Menu session started");
    return true;
}

bool PokemonDatabase::swapByPos(int boxX, int slotX, int boxY, int slotY) {
    assert(m_inMenuSession);
    if (boxX == boxY && slotX == slotY) return true;

    const bool xIsParty = (boxX == -1);
    const bool yIsParty = (boxY == -1);

    if (xIsParty && yIsParty) {
        std::swap(m_party[slotX], m_party[slotY]);
        return dbWritePartySlot(slotX, m_party[slotX])
             & dbWritePartySlot(slotY, m_party[slotY]);
    }

    if (!xIsParty && !yIsParty)
        return swapPCSlots(boxX, slotX, boxY, slotY);

    const int pcBox  = xIsParty ? boxY  : boxX;
    const int pcSlot = xIsParty ? slotY : slotX;
    const int pSlot  = xIsParty ? slotX : slotY;
    if (!isBoxLoaded(pcBox) && !loadBox(pcBox)) return false;
    std::swap(m_boxCache[pcBox][pcSlot], m_party[pSlot]);
    return dbWritePCSlot(pcBox, pcSlot, m_boxCache[pcBox][pcSlot])
         & dbWritePartySlot(pSlot, m_party[pSlot]);
}

bool PokemonDatabase::commitMenuSession() {
    assert(m_inMenuSession);
    if (!QSqlDatabase::database().commit()) {
        DB_ERR("Failed to commit transaction");
        return false;
    }
    m_inMenuSession = false;
    DB_LOG("Menu session committed");
    return true;
}

bool PokemonDatabase::rollbackMenuSession() {
    assert(m_inMenuSession);
    if (!QSqlDatabase::database().rollback()) {
        DB_ERR("Failed to rollback transaction");
        m_inMenuSession = false;
        return false;
    }
    m_boxCache.clear();
    if (!loadWildAndParty())
        DB_ERR("rollbackMenuSession: failed to reload wild/party after rollback");
    m_inMenuSession = false;
    DB_LOG("Menu session rolled back");
    return true;
}

bool PokemonDatabase::toggleExpShare() {
    Defaults d = loadDefaults();
    d.expShareOn = !d.expShareOn;
    return writeDefaults(d);
}

bool PokemonDatabase::isExpShareOn() const {
    return const_cast<PokemonDatabase*>(this)->loadDefaults().expShareOn;
}

std::vector<GameState> PokemonDatabase::listSaves() {
    std::vector<GameState> saves;
    QSqlQuery q("SELECT * FROM saves");
    while (q.next()) {
        saves.push_back({
            q.value("save_id").toInt(),
            q.value("player_sprite_id").toInt(),
            q.value("name").toString().toStdString(),
            q.value("current_box").toInt(),
            q.value("unlocked_boxes").toInt(),
        });
    }
    if (q.lastError().isValid()) logQuery(q);
    return saves;
}

bool PokemonDatabase::switchSave(int save_id) {
    DB_LOG("Switching to save_id=" << save_id);
    m_saveId        = save_id;
    m_boxCache.clear();
    m_inMenuSession = false;
    return initFixedSlots() && loadWildAndParty();
}

static bool patchSlot(int saveId, int box, int slot,
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
        if (!q.exec()) { logQuery(q); return false; }
        if (q.numRowsAffected() == 0)
            DB_WARN("patchSlot: no PC row at [box=" << box << " slot=" << slot << "] — patch skipped");
        return q.numRowsAffected() > 0;
    }
    bool ok = q.exec();
    if (!ok) logQuery(q);
    return ok;
}

PokemonState* PokemonDatabase::cachePtr(int box, int slot) {
    if (box == -2) return &m_wild;
    if (box == -1) return (slot >= 0 && slot < PARTY_SIZE) ? &m_party[slot] : nullptr;
    if (!isBoxLoaded(box) && !loadBox(box)) return nullptr;
    return (slot >= 0 && slot < BOX_SIZE) ? &m_boxCache[box][slot] : nullptr;
}

bool PokemonDatabase::renamePokemon(int box, int slot, const std::string& newName) {
    PokemonState* p = cachePtr(box, slot);
    if (!p || p->empty()) return false;
    p->name = newName;
    QString qName = QString::fromStdString(newName);
    bool ok = patchSlot(m_saveId, box, slot, "name=?", [&](QSqlQuery& q){ q.addBindValue(qName); });
    if (ok) DB_LOG("Renamed [box=" << box << " slot=" << slot << "] ->" << qName);
    return ok;
}

bool PokemonDatabase::setPokemonMoves(int box, int slot, const int moves[4]) {
    PokemonState* p = cachePtr(box, slot);
    if (!p || p->empty()) return false;
    for (int i = 0; i < 4; ++i) p->moves[i] = moves[i];
    return patchSlot(m_saveId, box, slot, "move0=?, move1=?, move2=?, move3=?",
                     [&](QSqlQuery& q){
                         q.addBindValue(moves[0]); q.addBindValue(moves[1]);
                         q.addBindValue(moves[2]); q.addBindValue(moves[3]);
                     });
}

bool PokemonDatabase::setPokemonMove(int box, int slot, int moveIndex, int moveId) {
    if (moveIndex < 0 || moveIndex > 3) return false;
    PokemonState* p = cachePtr(box, slot);
    if (!p || p->empty()) return false;
    p->moves[moveIndex] = moveId;
    static const char* cols[] = {"move0=?", "move1=?", "move2=?", "move3=?"};
    return patchSlot(m_saveId, box, slot, cols[moveIndex],
                     [&](QSqlQuery& q){ q.addBindValue(moveId); });
}

Defaults PokemonDatabase::loadDefaults() {
    Defaults d;
    QSqlQuery q;
    q.prepare("SELECT scale, speed, lvl_range_up, lvl_range_down, exp_share_on FROM defaults WHERE save_id=?");
    q.addBindValue(m_saveId);
    if (q.exec() && q.next()) {
        d.scale        = q.value("scale").toInt();
        d.speed        = q.value("speed").toInt();
        d.lvlRangeUp   = q.value("lvl_range_up").toInt();
        d.lvlRangeDown = q.value("lvl_range_down").toInt();
        d.expShareOn   = q.value("exp_share_on").toBool();
    } else { logQuery(q); }
    return d;
}

bool PokemonDatabase::writeDefaults(const Defaults& d) {
    QSqlQuery q;
    q.prepare(R"(UPDATE defaults SET
        scale=?, speed=?, lvl_range_up=?, lvl_range_down=?, exp_share_on=?
        WHERE save_id=?)");
    q.addBindValue(d.scale);
    q.addBindValue(d.speed);
    q.addBindValue(d.lvlRangeUp);
    q.addBindValue(d.lvlRangeDown);
    q.addBindValue(d.expShareOn ? 1 : 0);
    q.addBindValue(m_saveId);
    bool ok = q.exec();
    if (!ok) logQuery(q);
    else DB_LOG("Defaults written for save_id=" << m_saveId);
    return ok;
}

#include "PokemonDatabase.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <iostream>
#include <sstream>
#include <unordered_map>

PokemonDatabase& PokemonDatabase::instance() {
    static PokemonDatabase instance;
    return instance;
}

PokemonDatabase::~PokemonDatabase() {
    shutdown();
}

bool PokemonDatabase::initialize(const std::string& dbPath) {
    if (m_initialized) {
        if (!dbPath.empty() && dbPath != m_dbPath) {
            qWarning() << "Database already initialized with different path!";
            qWarning() << "Current:" << QString::fromStdString(m_dbPath);
            qWarning() << "Requested:" << QString::fromStdString(dbPath);
        }
        return true;
    }

    // If no path provided, use a default writable location
    if (dbPath.empty()) {
        QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(defaultDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        m_dbPath = (defaultDir + "/pokemon.db").toStdString();
        qDebug() << "No database path provided, using default:" << QString::fromStdString(m_dbPath);
    } else {
        m_dbPath = dbPath;
    }

    // Verify parent directory exists
    QString qDbPath = QString::fromStdString(m_dbPath);
    QFileInfo fileInfo(qDbPath);
    QDir parentDir = fileInfo.dir();

    if (!parentDir.exists()) {
        qWarning() << "Database directory does not exist:" << parentDir.path();
        if (!parentDir.mkpath(".")) {
            qCritical() << "Failed to create database directory:" << parentDir.path();
            return false;
        }
        qDebug() << "Created database directory:" << parentDir.path();
    }

    // Check if we have write permissions
    if (!QFileInfo(parentDir.path()).isWritable()) {
        qCritical() << "Database directory is not writable:" << parentDir.path();
        return false;
    }

    // Initialize Qt SQL database
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(qDbPath);

    if (!db.open()) {
        qCritical() << "Failed to open database:" << db.lastError().text();
        return false;
    }

    qDebug() << "Database opened successfully at:" << qDbPath;

    // Create tables if they don't exist
    createTables();

    // Ensure wild slot exists
    ensureWildSlotExists();

    m_initialized = true;
    return true;
}

void PokemonDatabase::shutdown() {
    if (m_initialized) {
        QSqlDatabase::database().close();
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
        m_initialized = false;
    }
}

void PokemonDatabase::createTables() {
    const char* pokemonTable = R"(
        CREATE TABLE IF NOT EXISTS pokemon (
            _id INTEGER PRIMARY KEY,
            pokedex_id INTEGER NOT NULL,
            variant_id INTEGER DEFAULT 0,
            pokeball_id INTEGER DEFAULT 0,
            name TEXT NOT NULL,
            lvl INTEGER DEFAULT 1,
            current_xp INTEGER DEFAULT 0,
            nature INTEGER,
            move1 INTEGER DEFAULT 0,
            move2 INTEGER DEFAULT 0,
            move3 INTEGER DEFAULT 0,
            move4 INTEGER DEFAULT 0,
            has_exp_share INTEGER DEFAULT 0,
            location_type INTEGER DEFAULT 1,
            box_number INTEGER DEFAULT 0,
            slot_index INTEGER DEFAULT 0
        )
    )";

    const char* gameStateTable = R"(
        CREATE TABLE IF NOT EXISTS game_state (
            _id INTEGER PRIMARY KEY DEFAULT 1,
            player_sprite_id INTEGER DEFAULT 0,
            name TEXT DEFAULT 'Player',
            current_box INTEGER DEFAULT 0,
            unlocked_boxes INTEGER DEFAULT 10
        )
    )";

    QSqlQuery query;
    if (!query.exec(pokemonTable)) {
        qCritical() << "Failed to create pokemon table:" << query.lastError().text();
    }
    if (!query.exec(gameStateTable)) {
        qCritical() << "Failed to create game_state table:" << query.lastError().text();
    }

    // Create index for faster location-based queries
    query.exec("CREATE INDEX IF NOT EXISTS idx_pokemon_location ON pokemon(location_type, box_number, slot_index)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_pokemon_box ON pokemon(box_number, slot_index)");
}

void PokemonDatabase::ensureWildSlotExists() {
    const char* sql = R"(
        INSERT OR IGNORE INTO pokemon (_id, pokedex_id, name, location_type)
        VALUES (0, 0, 'WILD_SLOT', -1)
    )";
    QSqlQuery query;
    query.exec(sql);
}

PokemonState PokemonDatabase::getPokemon(int id) {
    if (!m_initialized) return PokemonState{};

    QSqlQuery query;
    query.prepare("SELECT * FROM pokemon WHERE _id = ?");
    query.addBindValue(id);

    PokemonState result{};
    if (query.exec() && query.next()) {
        result = queryToPokemon(query);
    }

    return result;
}

PokemonState PokemonDatabase::queryToPokemon(const QSqlQuery& query) {
    PokemonState pokemon;

    pokemon._id = query.value(0).toInt();
    pokemon.pokedex_id = query.value(1).toInt();
    pokemon.variant_id = query.value(2).toInt();
    pokemon.pokeball_id = query.value(3).toInt();
    pokemon.name = query.value(4).toString().toStdString();
    pokemon.lvl = query.value(5).toInt();
    pokemon.currentXP = query.value(6).toInt();
    pokemon.nature = static_cast<Nature>(query.value(7).toInt());

    pokemon.moves[0] = query.value(8).toInt();
    pokemon.moves[1] = query.value(9).toInt();
    pokemon.moves[2] = query.value(10).toInt();
    pokemon.moves[3] = query.value(11).toInt();

    pokemon.hasExpShare = query.value(12).toBool();
    pokemon.location = static_cast<PokemonState::Location>(query.value(13).toInt());
    pokemon.box_number = query.value(14).toInt();
    pokemon.slot_index = query.value(15).toInt();

    return pokemon;
}

void PokemonDatabase::bindPokemonParams(QSqlQuery& query, const PokemonState& pokemon) {
    query.addBindValue(pokemon.pokedex_id);
    query.addBindValue(pokemon.variant_id);
    query.addBindValue(pokemon.pokeball_id);
    query.addBindValue(QString::fromStdString(pokemon.name));
    query.addBindValue(pokemon.lvl);
    query.addBindValue(pokemon.currentXP);
    query.addBindValue(static_cast<int>(pokemon.nature));

    for (int i = 0; i < 4; i++) {
        query.addBindValue(pokemon.moves[i]);
    }

    query.addBindValue(pokemon.hasExpShare ? 1 : 0);
    query.addBindValue(static_cast<int>(pokemon.location));
    query.addBindValue(pokemon.box_number);
    query.addBindValue(pokemon.slot_index);
}

int PokemonDatabase::createPokemon(const PokemonState& pokemon) {
    if (!m_initialized) return -1;

    // Get next available ID
    QSqlQuery query;
    query.prepare("SELECT COALESCE(MAX(_id), 0) FROM pokemon WHERE _id > 0");
    query.exec();
    query.next();
    int newId = query.value(0).toInt() + 1;

    query.prepare(R"(
        INSERT INTO pokemon VALUES (
            ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
        )
    )");

    query.addBindValue(newId);
    bindPokemonParams(query, pokemon);

    bool success = query.exec();
    return success ? newId : -1;
}

bool PokemonDatabase::updatePokemon(const PokemonState& pokemon) {
    if (!m_initialized || pokemon._id < 0) return false;

    QSqlQuery query;
    query.prepare(R"(
        UPDATE pokemon SET
            pokedex_id = ?, variant_id = ?, pokeball_id = ?, name = ?,
            lvl = ?, current_xp = ?, nature = ?,
            move1 = ?, move2 = ?, move3 = ?, move4 = ?,
            has_exp_share = ?, location_type = ?, box_number = ?, slot_index = ?
        WHERE _id = ?
    )");

    bindPokemonParams(query, pokemon);
    query.addBindValue(pokemon._id);

    return query.exec();
}

bool PokemonDatabase::batchUpdatePokemon(const std::vector<PokemonState>& updates) {
    if (!m_initialized || updates.empty()) return false;

    QSqlDatabase::database().transaction();

    bool success = true;
    QSqlQuery query;
    query.prepare(R"(
        UPDATE pokemon SET
            pokedex_id = ?, variant_id = ?, pokeball_id = ?, name = ?,
            lvl = ?, current_xp = ?, nature = ?,
            move1 = ?, move2 = ?, move3 = ?, move4 = ?,
            has_exp_share = ?, location_type = ?, box_number = ?, slot_index = ?
        WHERE _id = ?
    )");

    for (const auto& pokemon : updates) {
        if (pokemon._id < 0) continue;

        bindPokemonParams(query, pokemon);
        query.addBindValue(pokemon._id);

        if (!query.exec()) {
            qWarning() << "Failed to update Pokémon" << pokemon._id << ":" << query.lastError().text();
            success = false;
            break;
        }
    }

    if (success) {
        QSqlDatabase::database().commit();
    } else {
        QSqlDatabase::database().rollback();
    }

    return success;
}

bool PokemonDatabase::deletePokemon(int id) {
    if (!m_initialized || id <= 0) return false;

    QSqlQuery query;
    query.prepare("DELETE FROM pokemon WHERE _id = ?");
    query.addBindValue(id);

    return query.exec();
}

std::array<PokemonState, PARTY_SIZE> PokemonDatabase::getParty() {
    std::array<PokemonState, PARTY_SIZE> party;

    if (!m_initialized) {
        // Return empty party
        for (auto& poke : party) {
            poke._id = -1;
        }
        return party;
    }

    QSqlQuery query;
    query.prepare(R"(
        SELECT * FROM pokemon
        WHERE location_type = 0
        ORDER BY slot_index
        LIMIT ?
    )");
    query.addBindValue(PARTY_SIZE);

    if (query.exec()) {
        int slot = 0;
        while (query.next() && slot < PARTY_SIZE) {
            party[slot] = queryToPokemon(query);
            slot++;
        }

        // Fill remaining slots with empty Pokémon
        for (; slot < PARTY_SIZE; slot++) {
            party[slot]._id = -1;
        }
    } else {
        // Fill with empty Pokémon on error
        for (auto& poke : party) {
            poke._id = -1;
        }
    }

    return party;
}

std::array<PokemonState, BOX_SIZE> PokemonDatabase::getBox(int box_number) {
    std::array<PokemonState, BOX_SIZE> box;

    if (!m_initialized || box_number < 0) {
        // Return empty box
        for (auto& poke : box) {
            poke._id = -1;
        }
        return box;
    }

    QSqlQuery query;
    query.prepare(R"(
        SELECT * FROM pokemon
        WHERE location_type = 1 AND box_number = ?
        ORDER BY slot_index
        LIMIT ?
    )");
    query.addBindValue(box_number);
    query.addBindValue(BOX_SIZE);

    if (query.exec()) {
        int slot = 0;
        while (query.next() && slot < BOX_SIZE) {
            box[slot] = queryToPokemon(query);
            slot++;
        }

        // Fill remaining slots with empty Pokémon
        for (; slot < BOX_SIZE; slot++) {
            box[slot]._id = -1;
        }
    } else {
        // Fill with empty Pokémon on error
        for (auto& poke : box) {
            poke._id = -1;
        }
    }

    return box;
}

std::vector<PokemonState> PokemonDatabase::getPokemonInBox(int box_number) {
    std::vector<PokemonState> pokemonList;

    if (!m_initialized || box_number < 0) return pokemonList;

    QSqlQuery query;
    query.prepare(R"(
        SELECT * FROM pokemon
        WHERE location_type = 1 AND box_number = ?
        ORDER BY slot_index
    )");
    query.addBindValue(box_number);

    if (query.exec()) {
        while (query.next()) {
            pokemonList.push_back(queryToPokemon(query));
        }
    }

    return pokemonList;
}

std::vector<PokemonState> PokemonDatabase::getMultiplePokemon(const std::vector<int>& ids) {
    std::vector<PokemonState> results;
    if (!m_initialized || ids.empty()) return results;

    results.reserve(ids.size());

    // Handle wild Pokémon (ID 0) specially
    for (int id : ids) {
        if (id == 0) {
            results.push_back(getWildPokemon());
            continue;
        }
    }

    // Build batch query for remaining IDs
    std::vector<int> validIds;
    for (int id : ids) {
        if (id > 0) {
            validIds.push_back(id);
        }
    }

    if (validIds.empty()) return results;

    // Build IN clause
    std::string sql = "SELECT * FROM pokemon WHERE _id IN (";
    for (size_t i = 0; i < validIds.size(); i++) {
        sql += "?";
        if (i < validIds.size() - 1) sql += ",";
    }
    sql += ")";

    QSqlQuery query;
    query.prepare(QString::fromStdString(sql));

    for (int id : validIds) {
        query.addBindValue(id);
    }

    if (query.exec()) {
        std::unordered_map<int, PokemonState> fetchedMap;
        while (query.next()) {
            PokemonState poke = queryToPokemon(query);
            fetchedMap[poke._id] = poke;
        }

        // Match results to input order
        for (int id : ids) {
            if (id <= 0) continue; // Already handled wild Pokémon

            auto it = fetchedMap.find(id);
            if (it != fetchedMap.end()) {
                results.push_back(it->second);
            } else {
                // Pokémon not found, add empty
                results.push_back(PokemonState{});
                results.back()._id = -1;
            }
        }
    }

    return results;
}

bool PokemonDatabase::moveToParty(int pokemon_id, int party_slot) {
    if (!m_initialized || pokemon_id < 0 || party_slot < 0 || party_slot >= PARTY_SIZE) {
        return false;
    }

    // Check if party slot is occupied
    QSqlQuery checkQuery;
    checkQuery.prepare(R"(
        SELECT _id FROM pokemon
        WHERE location_type = 0 AND slot_index = ?
    )");
    checkQuery.addBindValue(party_slot);

    if (checkQuery.exec() && checkQuery.next()) {
        int occupyingId = checkQuery.value(0).toInt();
        // Move occupying Pokémon to a box
        if (!moveToBox(occupyingId, 0, getNextAvailableSlot(0))) {
            return false;
        }
    }

    // Update the Pokémon's location
    QSqlQuery query;
    query.prepare(R"(
        UPDATE pokemon SET
        location_type = 0, box_number = 0, slot_index = ?
        WHERE _id = ?
    )");
    query.addBindValue(party_slot);
    query.addBindValue(pokemon_id);

    return query.exec();
}

bool PokemonDatabase::moveToBox(int pokemon_id, int box_number, int box_slot) {
    if (!m_initialized || pokemon_id < 0 || box_number < 0 || box_slot < 0 || box_slot >= BOX_SIZE) {
        return false;
    }

    // Check if box slot is occupied
    QSqlQuery checkQuery;
    checkQuery.prepare(R"(
        SELECT _id FROM pokemon
        WHERE location_type = 1 AND box_number = ? AND slot_index = ?
    )");
    checkQuery.addBindValue(box_number);
    checkQuery.addBindValue(box_slot);

    if (checkQuery.exec() && checkQuery.next()) {
        int occupyingId = checkQuery.value(0).toInt();
        // Move occupying Pokémon to another slot
        if (!moveToBox(occupyingId, box_number, getNextAvailableSlot(box_number))) {
            return false;
        }
    }

    // Update the Pokémon's location
    QSqlQuery query;
    query.prepare(R"(
        UPDATE pokemon SET
        location_type = 1, box_number = ?, slot_index = ?
        WHERE _id = ?
    )");
    query.addBindValue(box_number);
    query.addBindValue(box_slot);
    query.addBindValue(pokemon_id);

    return query.exec();
}

bool PokemonDatabase::swapPokemon(int id1, int id2) {
    if (!m_initialized || id1 < 0 || id2 < 0) return false;

    // Get current locations
    PokemonState poke1 = getPokemon(id1);
    PokemonState poke2 = getPokemon(id2);

    if (poke1._id == -1 || poke2._id == -1) return false;

    // Swap locations
    std::swap(poke1.box_number, poke2.box_number);
    std::swap(poke1.slot_index, poke2.slot_index);

    // Update both
    return updatePokemon(poke1) && updatePokemon(poke2);
}

PokemonState PokemonDatabase::getWildPokemon() {
    return getPokemon(0);
}

void PokemonDatabase::spawnWildPokemon(const PokemonState& templatePokemon) {
    if (!m_initialized) return;

    PokemonState wild = templatePokemon;
    wild._id = 0;
    wild.location = PokemonState::WILD;
    wild.box_number = 0;
    wild.slot_index = 0;
    updatePokemon(wild);
}

int PokemonDatabase::catchWildPokemon(int pokeball_id) {
    if (!m_initialized) return -1;

    PokemonState wild = getPokemon(0);

    // Don't catch empty wild slot
    if (wild.pokedex_id == 0) return -1;

    wild.pokeball_id = pokeball_id;
    wild.location = PokemonState::BOX;
    wild.box_number = 0;
    wild.slot_index = getNextAvailableSlot(0);

    int caughtId = createPokemon(wild);
    if (caughtId <= 0) return -1;

    // Clear wild slot
    clearWild();

    return caughtId;
}

bool PokemonDatabase::clearWild() {
    if (!m_initialized) return false;

    QSqlQuery query;
    query.prepare(R"(
        UPDATE pokemon SET
            pokedex_id = 0, name = 'WILD_SLOT',
            lvl = 1, current_xp = 0, nature = 0,
            move1 = 0, move2 = 0, move3 = 0, move4 = 0,
            has_exp_share = 0, location_type = -1,
            box_number = 0, slot_index = 0
        WHERE _id = 0
    )");

    return query.exec();
}

GameState PokemonDatabase::loadGameState() {
    GameState state;

    if (!m_initialized) return state;

    QSqlQuery query;
    query.prepare("SELECT * FROM game_state WHERE _id = 1");

    if (query.exec() && query.next()) {
        state.player_sprite_id = query.value(1).toInt();
        state.name = query.value(2).toString().toStdString();
        state.current_box = query.value(3).toInt();
        state.unlocked_boxes = query.value(4).toInt();
    } else {
        // Create default game state if it doesn't exist
        saveGameState(state);
    }

    return state;
}

bool PokemonDatabase::saveGameState(const GameState& state) {
    if (!m_initialized) return false;

    QSqlQuery query;
    query.prepare(R"(
        INSERT OR REPLACE INTO game_state
        (_id, player_sprite_id, name, current_box, unlocked_boxes)
        VALUES (1, ?, ?, ?, ?)
    )");

    query.addBindValue(state.player_sprite_id);
    query.addBindValue(QString::fromStdString(state.name));
    query.addBindValue(state.current_box);
    query.addBindValue(state.unlocked_boxes);

    return query.exec();
}

int PokemonDatabase::countPokemonInBox(int box_number) {
    if (!m_initialized || box_number < 0) return 0;

    QSqlQuery query;
    query.prepare(R"(
        SELECT COUNT(*) FROM pokemon
        WHERE location_type = 1 AND box_number = ?
    )");
    query.addBindValue(box_number);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int PokemonDatabase::getNextAvailableSlot(int box_number) {
    if (!m_initialized || box_number < 0) return -1;

    QSqlQuery query;
    query.prepare(R"(
        WITH RECURSIVE
        slots(slot) AS (
            SELECT 0
            UNION ALL
            SELECT slot + 1 FROM slots WHERE slot < ?
        )
        SELECT s.slot
        FROM slots s
        LEFT JOIN pokemon p ON p.location_type = 1
            AND p.box_number = ?
            AND p.slot_index = s.slot
        WHERE p._id IS NULL
        ORDER BY s.slot
        LIMIT 1
    )");
    query.addBindValue(BOX_SIZE - 1);  // Max slot index
    query.addBindValue(box_number);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

bool PokemonDatabase::toggleExpShare(int pokemon_id) {
    if (!m_initialized || pokemon_id <= 0) return false;

    QSqlQuery query;
    query.prepare(R"(
        UPDATE pokemon SET has_exp_share = NOT has_exp_share
        WHERE _id = ?
    )");
    query.addBindValue(pokemon_id);

    return query.exec();
}

std::vector<int> PokemonDatabase::getPartyExpShareIds() {
    std::vector<int> expShareIds;

    if (!m_initialized) return expShareIds;

    QSqlQuery query;
    query.prepare(R"(
        SELECT _id FROM pokemon
        WHERE location_type = 0 AND has_exp_share = 1
    )");

    if (query.exec()) {
        while (query.next()) {
            expShareIds.push_back(query.value(0).toInt());
        }
    }

    return expShareIds;
}

void PokemonDatabase::updateLocation(int id, PokemonState::Location loc, int box_num, int slot_idx) {
    if (!m_initialized || id < 0) return;

    QSqlQuery query;
    query.prepare(R"(
        UPDATE pokemon SET
        location_type = ?, box_number = ?, slot_index = ?
        WHERE _id = ?
    )");
    query.addBindValue(static_cast<int>(loc));
    query.addBindValue(box_num);
    query.addBindValue(slot_idx);
    query.addBindValue(id);

    query.exec();
}

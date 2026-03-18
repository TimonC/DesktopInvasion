#include "PokemonDatabase.h"
#include "sqlite3.h"
#include <iostream>
#include <sstream>
#include <unordered_set>

PokemonDatabase& PokemonDatabase::instance() {
    static PokemonDatabase instance;
    return instance;
}

PokemonDatabase::~PokemonDatabase() {
    shutdown();
}

bool PokemonDatabase::initialize(const std::string& dbPath) {
    if (m_db) return true;

    m_dbPath = dbPath;

    if (sqlite3_open(dbPath.c_str(), &m_db) != SQLITE_OK) {
        std::cerr << "Database error: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    sqlite3_exec(m_db, "PRAGMA foreign_keys = ON", nullptr, nullptr, nullptr);
    createTables();
    ensureWildSlotExists();

    return true;
}

void PokemonDatabase::shutdown() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

void PokemonDatabase::createTables() {
    const char* pokemonTable = R"(
        CREATE TABLE IF NOT EXISTS pokemon (
            _id INTEGER PRIMARY KEY,
            pokedex_id INTEGER NOT NULL,
            variant_id INTEGER DEFAULT 0,
            name TEXT NOT NULL,
            iv_hp INTEGER, iv_attack INTEGER, iv_defense INTEGER,
            iv_spattack INTEGER, iv_spdefense INTEGER, iv_speed INTEGER,
            ev_hp INTEGER DEFAULT 0, ev_attack INTEGER DEFAULT 0,
            ev_defense INTEGER DEFAULT 0, ev_spattack INTEGER DEFAULT 0,
            ev_spdefense INTEGER DEFAULT 0, ev_speed INTEGER DEFAULT 0,
            nature INTEGER,
            move1 INTEGER DEFAULT 0, move2 INTEGER DEFAULT 0,
            move3 INTEGER DEFAULT 0, move4 INTEGER DEFAULT 0,
            total_xp INTEGER DEFAULT 0
        )
    )";

    const char* gameStateTable = R"(
        CREATE TABLE IF NOT EXISTS game_state (
            _id INTEGER PRIMARY KEY DEFAULT 1,
            player_sprite_id INTEGER DEFAULT 0,
            name TEXT DEFAULT 'Player',
            party_ids TEXT DEFAULT '0,0,0,0,0,0'
        )
    )";

    sqlite3_exec(m_db, pokemonTable, nullptr, nullptr, nullptr);
    sqlite3_exec(m_db, gameStateTable, nullptr, nullptr, nullptr);
}

void PokemonDatabase::ensureWildSlotExists() {
    const char* sql = R"(
        INSERT OR IGNORE INTO pokemon (_id, pokedex_id, name)
        VALUES (0, 0, 'WILD_SLOT')
    )";
    sqlite3_exec(m_db, sql, nullptr, nullptr, nullptr);
}

PokemonState PokemonDatabase::getPokemon(int id) {
    if (!m_db) return PokemonState{};

    const char* sql = "SELECT * FROM pokemon WHERE _id = ?";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return PokemonState{};
    }

    sqlite3_bind_int(stmt, 1, id);

    PokemonState result{};
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = queryToPokemon(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

PokemonState PokemonDatabase::queryToPokemon(sqlite3_stmt* stmt) {
    PokemonState pokemon;

    pokemon._id = sqlite3_column_int(stmt, 0);
    pokemon.pokedex_id = sqlite3_column_int(stmt, 1);
    pokemon.variant_id = sqlite3_column_int(stmt, 2);

    const unsigned char* nameText = sqlite3_column_text(stmt, 3);
    if (nameText) {
        pokemon.name = reinterpret_cast<const char*>(nameText);
    }

    pokemon.ivs[0] = sqlite3_column_int(stmt, 4);
    pokemon.ivs[1] = sqlite3_column_int(stmt, 5);
    pokemon.ivs[2] = sqlite3_column_int(stmt, 6);
    pokemon.ivs[3] = sqlite3_column_int(stmt, 7);
    pokemon.ivs[4] = sqlite3_column_int(stmt, 8);
    pokemon.ivs[5] = sqlite3_column_int(stmt, 9);

    pokemon.evs[0] = sqlite3_column_int(stmt, 10);
    pokemon.evs[1] = sqlite3_column_int(stmt, 11);
    pokemon.evs[2] = sqlite3_column_int(stmt, 12);
    pokemon.evs[3] = sqlite3_column_int(stmt, 13);
    pokemon.evs[4] = sqlite3_column_int(stmt, 14);
    pokemon.evs[5] = sqlite3_column_int(stmt, 15);

    pokemon.nature = static_cast<Nature>(sqlite3_column_int(stmt, 16));

    pokemon.moves[0] = sqlite3_column_int(stmt, 17);
    pokemon.moves[1] = sqlite3_column_int(stmt, 18);
    pokemon.moves[2] = sqlite3_column_int(stmt, 19);
    pokemon.moves[3] = sqlite3_column_int(stmt, 20);

    pokemon.total_xp = sqlite3_column_int(stmt, 21);

    return pokemon;
}

void PokemonDatabase::bindPokemonParams(sqlite3_stmt* stmt, const PokemonState& pokemon, int startCol) {
    int col = startCol;
    sqlite3_bind_int(stmt, col++, pokemon.pokedex_id);
    sqlite3_bind_int(stmt, col++, pokemon.variant_id);
    sqlite3_bind_text(stmt, col++, pokemon.name.c_str(), -1, SQLITE_STATIC);

    for (int i = 0; i < 6; i++) {
        sqlite3_bind_int(stmt, col++, static_cast<int>(pokemon.ivs[i]));
    }

    for (int i = 0; i < 6; i++) {
        sqlite3_bind_int(stmt, col++, static_cast<int>(pokemon.evs[i]));
    }

    sqlite3_bind_int(stmt, col++, static_cast<int>(pokemon.nature));

    for (int i = 0; i < 4; i++) {
        sqlite3_bind_int(stmt, col++, pokemon.moves[i]);
    }

    sqlite3_bind_int(stmt, col++, pokemon.total_xp);
}

int PokemonDatabase::createPokemon(const PokemonState& pokemon) {
    if (!m_db) return -1;

    const char* maxIdSql = "SELECT COALESCE(MAX(_id), 0) FROM pokemon WHERE _id > 0";
    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(m_db, maxIdSql, -1, &stmt, nullptr);
    sqlite3_step(stmt);
    int newId = sqlite3_column_int(stmt, 0) + 1;
    sqlite3_finalize(stmt);

    const char* insertSql = R"(
        INSERT INTO pokemon VALUES (
            ?, ?, ?, ?,
            ?, ?, ?, ?, ?, ?,
            ?, ?, ?, ?, ?, ?,
            ?, ?, ?, ?, ?, ?
        )
    )";

    sqlite3_prepare_v2(m_db, insertSql, -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, newId);
    bindPokemonParams(stmt, pokemon, 2);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    return success ? newId : -1;
}

bool PokemonDatabase::updatePokemon(const PokemonState& pokemon) {
    if (!m_db || pokemon._id < 0) return false;

    const char* updateSql = R"(
        UPDATE pokemon SET
            pokedex_id = ?, variant_id = ?, name = ?,
            iv_hp = ?, iv_attack = ?, iv_defense = ?,
            iv_spattack = ?, iv_spdefense = ?, iv_speed = ?,
            ev_hp = ?, ev_attack = ?, ev_defense = ?,
            ev_spattack = ?, ev_spdefense = ?, ev_speed = ?,
            nature = ?, move1 = ?, move2 = ?, move3 = ?, move4 = ?,
            total_xp = ?
        WHERE _id = ?
    )";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(m_db, updateSql, -1, &stmt, nullptr);

    bindPokemonParams(stmt, pokemon, 1);
    sqlite3_bind_int(stmt, 22, pokemon._id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    return success;
}

bool PokemonDatabase::deletePokemon(int id) {
    if (!m_db || id <= 0) return false;

    const char* sql = "DELETE FROM pokemon WHERE _id = ?";
    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return success;
}

PokemonState PokemonDatabase::getWildPokemon() {
    return getPokemon(0);
}

void PokemonDatabase::spawnWildPokemon(const PokemonState& templatePokemon) {
    if (!m_db) return;

    PokemonState wild = templatePokemon;
    wild._id = 0;
    updatePokemon(wild);
}

int PokemonDatabase::catchWildPokemon() {
    if (!m_db) return -1;

    PokemonState wild = getPokemon(0);
    if (wild.pokedex_id == 0) return -1;

    int caughtId = createPokemon(wild);
    if (caughtId <= 0) return -1;

    return caughtId;
}

bool PokemonDatabase::clearWild() {
    if (!m_db) return false;

    const char* clearWild = R"(
        UPDATE pokemon SET
            pokedex_id = 0, name = 'WILD_SLOT',
            iv_hp = 0, iv_attack = 0, iv_defense = 0,
            iv_spattack = 0, iv_spdefense = 0, iv_speed = 0,
            ev_hp = 0, ev_attack = 0, ev_defense = 0,
            ev_spattack = 0, ev_spdefense = 0, ev_speed = 0,
            nature = 0, move1 = 0, move2 = 0, move3 = 0, move4 = 0,
            total_xp = 0
        WHERE _id = 0
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, clearWild, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);

    if (!success) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(m_db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

GameState PokemonDatabase::loadGameState() {
    GameState state;

    if (!m_db) return state;

    const char* sql = "SELECT * FROM game_state WHERE _id = 1";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return state;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        state.player_sprite_id = sqlite3_column_int(stmt, 1);

        const unsigned char* nameText = sqlite3_column_text(stmt, 2);
        if (nameText) {
            state.name = reinterpret_cast<const char*>(nameText);
        }

        const unsigned char* partyText = sqlite3_column_text(stmt, 3);
        if (partyText) {
            std::string partyStr(reinterpret_cast<const char*>(partyText));
            std::stringstream ss(partyStr);
            std::string token;
            int i = 0;
            while (std::getline(ss, token, ',') && i < 6) {
                state.party_id[i++] = std::stoi(token);
            }
        }
    }

    sqlite3_finalize(stmt);
    return state;
}

bool PokemonDatabase::saveGameState(const GameState& state) {
    if (!m_db) return false;

    std::string partyStr;
    for (int i = 0; i < 6; i++) {
        partyStr += std::to_string(state.party_id[i]);
        if (i < 5) partyStr += ",";
    }

    const char* sql = R"(
        INSERT OR REPLACE INTO game_state (_id, player_sprite_id, name, party_ids)
        VALUES (1, ?, ?, ?)
    )";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, state.player_sprite_id);
    sqlite3_bind_text(stmt, 2, state.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, partyStr.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    return success;
}

bool PokemonDatabase::setPartyPokemon(int slot, int pokemonId) {
    if (slot < 0 || slot >= 6) return false;

    GameState state = loadGameState();
    state.party_id[slot] = pokemonId;
    return saveGameState(state);
}

bool PokemonDatabase::clearPartySlot(int slot) {
    return setPartyPokemon(slot, 0);
}

// ============= NEW EFFICIENT UPDATE METHODS =============

bool PokemonDatabase::isValidField(const std::string& field) {
    static const std::unordered_set<std::string> validFields = {
        // Core fields
        "pokedex_id", "variant_id", "name", "nature", "total_xp",
        // IV fields
        "iv_hp", "iv_attack", "iv_defense", "iv_spattack", "iv_spdefense", "iv_speed",
        // EV fields
        "ev_hp", "ev_attack", "ev_defense", "ev_spattack", "ev_spdefense", "ev_speed",
        // Move fields
        "move1", "move2", "move3", "move4"
    };
    return validFields.find(field) != validFields.end();
}

bool PokemonDatabase::executeUpdate(const std::string& sql, const std::vector<int>& params) {
    if (!m_db) return false;

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    for (size_t i = 0; i < params.size(); i++) {
        sqlite3_bind_int(stmt, i + 1, params[i]);
    }

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    return success;
}

bool PokemonDatabase::updatePokemonField(int pokemonId, const std::string& field, int value) {
    if (!isValidField(field)) {
        std::cerr << "Invalid field: " << field << std::endl;
        return false;
    }

    std::string sql = "UPDATE pokemon SET " + field + " = ? WHERE _id = ?";
    return executeUpdate(sql, {value, pokemonId});
}

bool PokemonDatabase::incrementPokemonField(int pokemonId, const std::string& field, int amount) {
    if (!isValidField(field)) {
        std::cerr << "Invalid field: " << field << std::endl;
        return false;
    }

    std::string sql = "UPDATE pokemon SET " + field + " = " + field + " + ? WHERE _id = ?";
    return executeUpdate(sql, {amount, pokemonId});
}

bool PokemonDatabase::addPokemonXp(int pokemonId, int xpAmount) {
    return incrementPokemonField(pokemonId, "total_xp", xpAmount);
}

int PokemonDatabase::getPokemonXp(int pokemonId) {
    if (!m_db || pokemonId <= 0) return -1;

    const char* sql = "SELECT total_xp FROM pokemon WHERE _id = ?";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, pokemonId);

    int xp = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        xp = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return xp;
}


bool PokemonDatabase::updatePokemonName(int pokemonId, const std::string& newName) {
    if (!m_db || pokemonId <= 0) return false;

    const char* sql = "UPDATE pokemon SET name = ? WHERE _id = ?";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, newName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, pokemonId);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    return success;
}

bool PokemonDatabase::updatePokemonFields(int pokemonId, const std::vector<std::pair<std::string, int>>& updates) {
    if (updates.empty() || !m_db || pokemonId <= 0) return false;

    // Build SQL
    std::string sql = "UPDATE pokemon SET ";
    std::vector<int> params;

    for (size_t i = 0; i < updates.size(); i++) {
        const auto& [field, value] = updates[i];
        if (!isValidField(field)) {
            std::cerr << "Invalid field in batch update: " << field << std::endl;
            return false;
        }
        sql += field + " = ?";
        params.push_back(value);
        if (i < updates.size() - 1) sql += ", ";
    }

    sql += " WHERE _id = ?";
    params.push_back(pokemonId);

    return executeUpdate(sql, params);
}

bool PokemonDatabase::incrementPokemonFields(int pokemonId, const std::vector<std::pair<std::string, int>>& increments) {
    if (increments.empty() || !m_db || pokemonId <= 0) return false;

    // Build SQL
    std::string sql = "UPDATE pokemon SET ";
    std::vector<int> params;

    for (size_t i = 0; i < increments.size(); i++) {
        const auto& [field, amount] = increments[i];
        if (!isValidField(field)) {
            std::cerr << "Invalid field in batch increment: " << field << std::endl;
            return false;
        }
        sql += field + " = " + field + " + ?";
        params.push_back(amount);
        if (i < increments.size() - 1) sql += ", ";
    }

    sql += " WHERE _id = ?";
    params.push_back(pokemonId);

    return executeUpdate(sql, params);
}

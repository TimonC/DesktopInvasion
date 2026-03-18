#include "PokemonDatabase.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
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
        // Already initialized - just verify path matches if provided
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
            iv_hp INTEGER, iv_attack INTEGER, iv_defense INTEGER,
            iv_spattack INTEGER, iv_spdefense INTEGER, iv_speed INTEGER,
            ev_hp INTEGER DEFAULT 0, ev_attack INTEGER DEFAULT 0,
            ev_defense INTEGER DEFAULT 0, ev_spattack INTEGER DEFAULT 0,
            ev_spdefense INTEGER DEFAULT 0, ev_speed INTEGER DEFAULT 0,
            nature INTEGER,
            move1 INTEGER DEFAULT 0, move2 INTEGER DEFAULT 0,
            move3 INTEGER DEFAULT 0, move4 INTEGER DEFAULT 0
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

    QSqlQuery query;
    query.exec(pokemonTable);
    query.exec(gameStateTable);
}

void PokemonDatabase::ensureWildSlotExists() {
    const char* sql = R"(
        INSERT OR IGNORE INTO pokemon (_id, pokedex_id, name)
        VALUES (0, 0, 'WILD_SLOT')
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

std::vector<PokemonState> PokemonDatabase::getPokemonBatch(const std::vector<int>& ids) {
    std::vector<PokemonState> results;
    if (!m_initialized || ids.empty()) return results;

    results.resize(ids.size());

    std::vector<std::pair<int, size_t>> validIdsWithIndex;
    for (size_t i = 0; i < ids.size(); i++) {
        if (ids[i] >= 0) {
            validIdsWithIndex.push_back({ids[i], i});
        }
    }

    if (validIdsWithIndex.empty()) return results;

    const size_t BATCH_SIZE = 56;

    for (size_t offset = 0; offset < validIdsWithIndex.size(); offset += BATCH_SIZE) {
        size_t count = std::min(BATCH_SIZE, validIdsWithIndex.size() - offset);

        std::string sql = "SELECT * FROM pokemon WHERE _id IN (";
        for (size_t i = 0; i < count; i++) {
            sql += "?";
            if (i < count - 1) sql += ",";
        }
        sql += ")";

        QSqlQuery query;
        query.prepare(QString::fromStdString(sql));

        for (size_t i = 0; i < count; i++) {
            query.addBindValue(validIdsWithIndex[offset + i].first);
        }

        if (!query.exec()) continue;

        std::unordered_map<int, PokemonState> fetchedMap;
        while (query.next()) {
            PokemonState poke = queryToPokemon(query);
            fetchedMap[poke._id] = poke;
        }

        for (size_t i = offset; i < offset + count; i++) {
            auto it = fetchedMap.find(validIdsWithIndex[i].first);
            if (it != fetchedMap.end()) {
                results[validIdsWithIndex[i].second] = it->second;
            }
        }
    }

    return results;
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

    pokemon.ivs[0] = query.value(7).toInt();
    pokemon.ivs[1] = query.value(8).toInt();
    pokemon.ivs[2] = query.value(9).toInt();
    pokemon.ivs[3] = query.value(10).toInt();
    pokemon.ivs[4] = query.value(11).toInt();
    pokemon.ivs[5] = query.value(12).toInt();

    pokemon.evs[0] = query.value(13).toInt();
    pokemon.evs[1] = query.value(14).toInt();
    pokemon.evs[2] = query.value(15).toInt();
    pokemon.evs[3] = query.value(16).toInt();
    pokemon.evs[4] = query.value(17).toInt();
    pokemon.evs[5] = query.value(18).toInt();

    pokemon.nature = static_cast<Nature>(query.value(19).toInt());

    pokemon.moves[0] = query.value(20).toInt();
    pokemon.moves[1] = query.value(21).toInt();
    pokemon.moves[2] = query.value(22).toInt();
    pokemon.moves[3] = query.value(23).toInt();

    return pokemon;
}

void PokemonDatabase::bindPokemonParams(QSqlQuery& query, const PokemonState& pokemon) {
    query.addBindValue(pokemon.pokedex_id);
    query.addBindValue(pokemon.variant_id);
    query.addBindValue(pokemon.pokeball_id);
    query.addBindValue(QString::fromStdString(pokemon.name));
    query.addBindValue(pokemon.lvl);
    query.addBindValue(pokemon.currentXP);

    for (int i = 0; i < 6; i++) {
        query.addBindValue(static_cast<int>(pokemon.ivs[i]));
    }

    for (int i = 0; i < 6; i++) {
        query.addBindValue(static_cast<int>(pokemon.evs[i]));
    }

    query.addBindValue(static_cast<int>(pokemon.nature));

    for (int i = 0; i < 4; i++) {
        query.addBindValue(pokemon.moves[i]);
    }
}

int PokemonDatabase::createPokemon(const PokemonState& pokemon) {
    if (!m_initialized) return -1;

    QSqlQuery query;
    query.prepare("SELECT COALESCE(MAX(_id), 0) FROM pokemon WHERE _id > 0");
    query.exec();
    query.next();
    int newId = query.value(0).toInt() + 1;

    query.prepare(R"(
        INSERT INTO pokemon VALUES (
            ?, ?, ?, ?, ?, ?, ?, ?,
            ?, ?, ?, ?, ?, ?, ?, ?,
            ?, ?, ?, ?, ?, ?, ?, ?
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
            lvl = ?, current_xp = ?,
            iv_hp = ?, iv_attack = ?, iv_defense = ?,
            iv_spattack = ?, iv_spdefense = ?, iv_speed = ?,
            ev_hp = ?, ev_attack = ?, ev_defense = ?,
            ev_spattack = ?, ev_spdefense = ?, ev_speed = ?,
            nature = ?, move1 = ?, move2 = ?, move3 = ?, move4 = ?
        WHERE _id = ?
    )");

    bindPokemonParams(query, pokemon);
    query.addBindValue(pokemon._id);

    return query.exec();
}

PokemonState PokemonDatabase::getWildPokemon() {
    return getPokemon(0);
}

void PokemonDatabase::spawnWildPokemon(const PokemonState& templatePokemon) {
    if (!m_initialized) return;

    PokemonState wild = templatePokemon;
    wild._id = 0;
    updatePokemon(wild);
}

int PokemonDatabase::catchWildPokemon(int pokeball_id) {
    if (!m_initialized) return -1;

    PokemonState wild = getPokemon(0);

    wild.pokeball_id = pokeball_id;
    int caughtId = createPokemon(wild);
    if (caughtId <= 0) return -1;

    return caughtId;
}

bool PokemonDatabase::clearWild() {
    if (!m_initialized) return false;

    QSqlQuery query;
    query.prepare(R"(
        UPDATE pokemon SET
            pokedex_id = 0, name = 'WILD_SLOT',
            lvl = 1, current_xp = 0,
            iv_hp = 0, iv_attack = 0, iv_defense = 0,
            iv_spattack = 0, iv_spdefense = 0, iv_speed = 0,
            ev_hp = 0, ev_attack = 0, ev_defense = 0,
            ev_spattack = 0, ev_spdefense = 0, ev_speed = 0,
            nature = 0, move1 = 0, move2 = 0, move3 = 0, move4 = 0
        WHERE _id = 0
    )");

    bool success = query.exec();

    if (!success) {
        std::cerr << "Failed to execute statement: " << query.lastError().text().toStdString() << std::endl;
    }

    return success;
}

GameState PokemonDatabase::loadGameState() {
    GameState state;

    if (!m_initialized) return state;

    QSqlQuery query;
    query.prepare("SELECT * FROM game_state WHERE _id = 1");

    if (!query.exec()) {
        return state;
    }

    if (query.next()) {
        state.player_sprite_id = query.value(1).toInt();
        state.name = query.value(2).toString().toStdString();

        QString partyStr = query.value(3).toString();
        std::stringstream ss(partyStr.toStdString());
        std::string token;
        int i = 0;
        while (std::getline(ss, token, ',') && i < 6) {
            state.party_id[i++] = std::stoi(token);
        }
    }

    return state;
}

bool PokemonDatabase::saveGameState(const GameState& state) {
    if (!m_initialized) return false;

    std::string partyStr;
    for (int i = 0; i < 6; i++) {
        partyStr += std::to_string(state.party_id[i]);
        if (i < 5) partyStr += ",";
    }

    QSqlQuery query;
    query.prepare(R"(
        INSERT OR REPLACE INTO game_state (_id, player_sprite_id, name, party_ids)
        VALUES (1, ?, ?, ?)
    )");

    query.addBindValue(state.player_sprite_id);
    query.addBindValue(QString::fromStdString(state.name));
    query.addBindValue(QString::fromStdString(partyStr));

    return query.exec();
}

bool PokemonDatabase::setPartyPokemon(int slot, int pokemonId) {
    if (slot < 0 || slot >= 6) return false;

    GameState state = loadGameState();
    state.party_id[slot] = pokemonId;
    return saveGameState(state);
}

bool PokemonDatabase::batchUpdatePokemon(const std::vector<PokemonState>& updates) {
    if (!m_initialized || updates.empty()) return false;

    QSqlDatabase::database().transaction();

    bool success = true;
    QSqlQuery query;
    query.prepare(R"(
        UPDATE pokemon SET
            pokedex_id = ?, variant_id = ?, pokeball_id = ?, name = ?,
            lvl = ?, current_xp = ?,
            iv_hp = ?, iv_attack = ?, iv_defense = ?,
            iv_spattack = ?, iv_spdefense = ?, iv_speed = ?,
            ev_hp = ?, ev_attack = ?, ev_defense = ?,
            ev_spattack = ?, ev_spdefense = ?, ev_speed = ?,
            nature = ?, move1 = ?, move2 = ?, move3 = ?, move4 = ?
        WHERE _id = ?
    )");

    for (const auto& pokemon : updates) {
        if (pokemon._id < 0) continue;

        bindPokemonParams(query, pokemon);
        query.addBindValue(pokemon._id);

        if (!query.exec()) {
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

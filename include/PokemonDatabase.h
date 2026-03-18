#ifndef POKEMONDATABASE_H
#define POKEMONDATABASE_H

#include <string>
#include "GameState.h"

struct sqlite3;
struct sqlite3_stmt;

class PokemonDatabase {
public:
    static PokemonDatabase& instance();

    bool initialize(const std::string& dbPath = "pokemon_game.db");
    void shutdown();

    PokemonState getPokemon(int id);
    int createPokemon(const PokemonState& pokemon);
    bool updatePokemon(const PokemonState& pokemon);
    bool deletePokemon(int id);

    PokemonState getWildPokemon();
    void spawnWildPokemon(const PokemonState& templatePokemon);
    int catchWildPokemon();

    GameState loadGameState();
    bool saveGameState(const GameState& state);

    bool setPartyPokemon(int slot, int pokemonId);
    bool clearPartySlot(int slot);

private:
    PokemonDatabase() = default;
    ~PokemonDatabase();

    PokemonDatabase(const PokemonDatabase&) = delete;
    PokemonDatabase& operator=(const PokemonDatabase&) = delete;

    void createTables();
    void ensureWildSlotExists();
    PokemonState queryToPokemon(sqlite3_stmt* stmt);
    void bindPokemonParams(sqlite3_stmt* stmt, const PokemonState& pokemon, int startCol = 1);

    sqlite3* m_db = nullptr;
    std::string m_dbPath;
};

#endif

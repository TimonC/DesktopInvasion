#ifndef POKEMONDATABASE_H
#define POKEMONDATABASE_H

#include <PokeTypes.h>
#include <string>
#include <vector>
#include <utility>

struct sqlite3;
struct sqlite3_stmt;

struct PokemonState {
    int _id = -1;
    int pokedex_id;
    int variant_id = 0;
    int pokeball_id = 0;
    std::string name;

    int lvl = 1;
    int currentXP = 0;

    int ivs[6];
    int evs[6];
    Nature nature;

    int moves[4] = {-1, -1, -1, -1};
};

struct GameState {
    int _id = 1;
    int player_sprite_id = 0;
    std::string name = "Player";
    int party_id[6] = {0, 0, 0, 0, 0, 0};
};

class PokemonDatabase {
public:
    static PokemonDatabase& instance();

    bool initialize(const std::string& dbPath = "/app/data/pokemon_game.db");
    void shutdown();

    // Core Pokemon operations
    PokemonState getPokemon(int id);
    std::vector<PokemonState> getPokemonBatch(const std::vector<int>& ids);

    int createPokemon(const PokemonState& pokemon);
    bool updatePokemon(const PokemonState& pokemon);
    bool deletePokemon(int id);

    // Efficient partial updates
    bool updatePokemonField(int pokemonId, const std::string& field, int value);
    bool incrementPokemonField(int pokemonId, const std::string& field, int amount);

    // Common operations as convenience methods
    bool addPokemonXp(int pokemonId, int xpAmount);
    int getPokemonXp(int pokemonId);
    bool setPokemonLevel(int pokemonId, int level);
    int getPokemonLevel(int pokemonId);
    bool updatePokemonName(int pokemonId, const std::string& newName);

    // Batch updates
    bool updatePokemonFields(int pokemonId, const std::vector<std::pair<std::string, int>>& updates);
    bool incrementPokemonFields(int pokemonId, const std::vector<std::pair<std::string, int>>& increments);

    // Wild Pokemon operations
    PokemonState getWildPokemon();
    void spawnWildPokemon(const PokemonState& templatePokemon);
    int catchWildPokemon(int pokeball_id);

    // Game state operations
    GameState loadGameState();
    bool saveGameState(const GameState& state);

    // Party management
    bool setPartyPokemon(int slot, int pokemonId);
    bool clearPartySlot(int slot);

    bool clearWild();

    bool batchUpdatePokemon(const std::vector<PokemonState>& updates);

private:
    PokemonDatabase() = default;
    ~PokemonDatabase();

    PokemonDatabase(const PokemonDatabase&) = delete;
    PokemonDatabase& operator=(const PokemonDatabase&) = delete;

    void createTables();
    void ensureWildSlotExists();
    PokemonState queryToPokemon(sqlite3_stmt* stmt);
    void bindPokemonParams(sqlite3_stmt* stmt, const PokemonState& pokemon, int startCol = 1);

    // Helper methods
    bool isValidField(const std::string& field);
    bool executeUpdate(const std::string& sql, const std::vector<int>& params);

    sqlite3* m_db = nullptr;
    std::string m_dbPath;
};

#endif

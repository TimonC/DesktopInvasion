#ifndef POKEMONDATABASE_H
#define POKEMONDATABASE_H
#include <PokeTypes.h>
#include <string>
#include <vector>
#include <QSqlQuery>

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
    bool initialize(const std::string& dbPath = "");
    void shutdown();
    PokemonState getPokemon(int id);
    std::vector<PokemonState> getPokemonBatch(const std::vector<int>& ids);
    int createPokemon(const PokemonState& pokemon);
    bool updatePokemon(const PokemonState& pokemon);
    PokemonState getWildPokemon();
    void spawnWildPokemon(const PokemonState& templatePokemon);
    int catchWildPokemon(int pokeball_id);
    GameState loadGameState();
    bool saveGameState(const GameState& state);
    bool setPartyPokemon(int slot, int pokemonId);
    bool clearWild();
    bool batchUpdatePokemon(const std::vector<PokemonState>& updates);

private:
    PokemonDatabase() = default;
    ~PokemonDatabase();
    PokemonDatabase(const PokemonDatabase&) = delete;
    PokemonDatabase& operator=(const PokemonDatabase&) = delete;
    void createTables();
    void ensureWildSlotExists();
    PokemonState queryToPokemon(const QSqlQuery& query);
    void bindPokemonParams(QSqlQuery& query, const PokemonState& pokemon);
    bool m_initialized = false;
    std::string m_dbPath;
};
#endif

#ifndef POKEMONDATABASE_H
#define POKEMONDATABASE_H
#include <PokeTypes.h>
#include <string>
#include <array>
#include <vector>
#include <QSqlQuery>

// Constants for fixed sizes
constexpr int PARTY_SIZE = 6;
constexpr int BOX_SIZE = 20;
constexpr int MAX_BOXES = 50;  // Example: 50 boxes * 20 = 1000 Pokémon capacity

struct PokemonState {
    int _id = -1;
    int pokedex_id;
    int variant_id = 0;
    int pokeball_id = 0;
    std::string name;
    int lvl = 1;
    int currentXP = 0;
    Nature nature;
    int moves[4] = {-1, -1, -1, -1};

    // EXP Share flag
    bool hasExpShare = false;

    // Location tracking
    enum Location {
        WILD = -1,
        PARTY = 0,
        BOX = 1
    };
    Location location = BOX;
    int box_number = 0;      // 0 for party, 1+ for boxes
    int slot_index = 0;      // 0-5 for party, 0-19 for box
};

struct GameState {
    int _id = 1;
    int player_sprite_id = 0;
    std::string name = "Player";
    int current_box = 0;     // Which box is currently being viewed
    int unlocked_boxes = 10; // How many boxes are available
};

class PokemonDatabase {
public:
    static PokemonDatabase& instance();

    // Core database operations
    bool initialize(const std::string& dbPath = "");
    void shutdown();

    // Pokémon operations
    PokemonState getPokemon(int id);
    int createPokemon(const PokemonState& pokemon);
    bool updatePokemon(const PokemonState& pokemon);
    bool deletePokemon(int id);

    // Batch operations
    bool batchUpdatePokemon(const std::vector<PokemonState>& updates);
    std::vector<PokemonState> getPokemonInBox(int box_number);
    std::vector<PokemonState> getMultiplePokemon(const std::vector<int>& ids);

    // UI-friendly operations
    std::array<PokemonState, PARTY_SIZE> getParty();
    std::array<PokemonState, BOX_SIZE> getBox(int box_number);

    // Location-based operations
    bool moveToParty(int pokemon_id, int party_slot);
    bool moveToBox(int pokemon_id, int box_number, int box_slot);
    bool swapPokemon(int id1, int id2);

    // Special slots
    PokemonState getWildPokemon();
    void spawnWildPokemon(const PokemonState& templatePokemon);
    int catchWildPokemon(int pokeball_id);
    bool clearWild();

    // Game state
    GameState loadGameState();
    bool saveGameState(const GameState& state);

    // Convenience
    int countPokemonInBox(int box_number);
    int getNextAvailableSlot(int box_number);
    bool toggleExpShare(int pokemon_id);
    std::vector<int> getPartyExpShareIds();

private:
    PokemonDatabase() = default;
    ~PokemonDatabase();
    PokemonDatabase(const PokemonDatabase&) = delete;
    PokemonDatabase& operator=(const PokemonDatabase&) = delete;

    void createTables();
    void ensureWildSlotExists();
    PokemonState queryToPokemon(const QSqlQuery& query);
    void bindPokemonParams(QSqlQuery& query, const PokemonState& pokemon);
    void updateLocation(int id, PokemonState::Location loc, int box_num, int slot_idx);

    bool m_initialized = false;
    std::string m_dbPath;
};
#endif

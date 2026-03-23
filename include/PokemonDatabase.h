#ifndef POKEMONDATABASE_H
#define POKEMONDATABASE_H

#include <PokeTypes.h>
#include <string>
#include <array>
#include <unordered_map>
#include <QSqlQuery>

constexpr int PARTY_SIZE = 6;
constexpr int BOX_SIZE   = 16;
constexpr int MAX_BOXES  = 99;

struct PokemonState {
    int         pokedex_id  = 0;
    int         variant_id  = 0;
    int         pokeball_id = 0;
    std::string name        = "";
    int         lvl         = 1;
    int         currentXP   = 0;
    Nature      nature      = Nature::Hardy;
    int         moves[4]    = {0};
    bool empty() const { return pokedex_id == 0; }
};

struct GameState {
    int         save_id          = 1;
    int         player_sprite_id = 0;
    std::string name             = "Player";
    int         current_box      = 0;
    int         unlocked_boxes   = 10;
};

struct Defaults {
    int  scale        = 2;
    int  speed        = 1;
    int  lvlRangeUp   = 5;
    int  lvlRangeDown = 5;
    bool expShareOn   = false;
};

class PokemonDatabase {
public:
    static PokemonDatabase& instance();

    int initialize(); //-1 on error, 0 on first play, 1 on success
    void shutdown();

    int  currentSaveId() const { return m_saveId; }
    bool setCurrentSaveId(int save_id);

    const PokemonState& wild() const { return m_wild; }
    void setWild(const PokemonState& p);
    void clearWild();

    const std::array<PokemonState, PARTY_SIZE>& party() const { return m_party; }
    void setPartySlot(int slot, const PokemonState& p);
    int  partySize()          const;
    int  firstFreePartySlot() const;

    bool                                      loadBox(int box);
    bool                                      isBoxLoaded(int box) const;
    const std::array<PokemonState, BOX_SIZE>& getBox(int box)      const;
    bool                                      setPCSlot(int box, int slot, const PokemonState& p);

    std::pair<int, int> catchWildPokemon(int pokeball_id);

    GameState loadGameState();
    bool      saveGameState(const GameState& state);

    bool swapByPos(int boxX, int slotX, int boxY, int slotY);

    bool toggleExpShare();
    bool isExpShareOn() const;

    bool renamePokemon  (int box, int slot, const std::string& newName);
    bool setPokemonMoves(int box, int slot, const int moves[4]);
    bool setPokemonMove (int box, int slot, int moveIndex, int moveId);

    Defaults loadDefaults();
    bool     writeDefaults(const Defaults& d);

private:
    PokemonDatabase() = default;
    ~PokemonDatabase();
    PokemonDatabase(const PokemonDatabase&)            = delete;
    PokemonDatabase& operator=(const PokemonDatabase&) = delete;

    bool createTables();
    bool initFixedSlots();
    bool loadWildAndParty();

    int  readCurrentSaveId();
    bool writeCurrentSaveId(int save_id);

    PokemonState rowToPokemon     (const QSqlQuery& q);
    void         writePokemonToRow(QSqlQuery& q, const PokemonState& p);

    bool dbWriteWild     (const PokemonState& p);
    bool dbWritePartySlot(int slot, const PokemonState& p);
    bool dbWritePCSlot   (int box, int slot, const PokemonState& p);
    bool dbDeletePCSlot  (int box, int slot);

    std::pair<int, int> firstFreePC();

    PokemonState* cachePtr(int box, int slot);

    bool    m_initialized = false;
    int     m_saveId      = 1;
    QString m_dbPath;

    PokemonState                                                m_wild;
    std::array<PokemonState, PARTY_SIZE>                        m_party;
    std::unordered_map<int, std::array<PokemonState, BOX_SIZE>> m_boxCache;
};

#endif

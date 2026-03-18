#ifndef POKEMONDATABASE_H
#define POKEMONDATABASE_H

#include <PokeTypes.h>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <QSqlQuery>

constexpr int PARTY_SIZE = 6;
constexpr int BOX_SIZE   = 16;
constexpr int MAX_BOXES  = 99;

struct PokemonState {
    int         pokedex_id  = 0;
    int         variant_id  = 0;
    int         pokeball_id = 0;
    std::string name;
    int         lvl         = 1;
    int         currentXP   = 0;
    Nature      nature      = Nature::Hardy;
    int         moves[4]    = {0, 0, 0, 0};

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
    int scale = 1;
    int speed = 1;
    int lvlRangeUp = 5;
    int lvlRangeDown = 5;
    bool expShareOn = false;
};

class PokemonDatabase {
public:
    static PokemonDatabase& instance();

    bool initialize(const std::string& dbPath = "", int save_id = 1);
    void shutdown();

    const PokemonState& wild() const { return m_wild; }
    void setWild(const PokemonState& p);
    void clearWild();

    const std::array<PokemonState, PARTY_SIZE>& party() const { return m_party; }
    void setPartySlot(int slot, const PokemonState& p);
    int  partySize()          const;
    int  firstFreePartySlot() const;

    void                                      loadBox(int box);
    bool                                      isBoxLoaded(int box) const;
    const std::array<PokemonState, BOX_SIZE>& getBox(int box)      const;
    void                                      setPCSlot(int box, int slot, const PokemonState& p);
    void                                      swapPCSlots(int boxA, int slotA, int boxB, int slotB);

    std::pair<int, int> catchWildPokemon(int pokeball_id);

    GameState loadGameState();
    bool      saveGameState(const GameState& state);

    void beginMenuSession();
    void swapByPos(int boxX, int slotX, int boxY, int slotY);
    void commitMenuSession();
    void rollbackMenuSession();

    bool toggleExpShare();
    bool isExpShareOn() const;

    std::vector<GameState> listSaves();
    bool                   switchSave(int save_id);

    void renamePokemon (int box, int slot, const std::string& newName);
    void setPokemonMoves(int box, int slot, const int moves[4]);
    void setPokemonMove (int box, int slot, int moveIndex, int moveId);

    Defaults loadDefaults();
    void writeDefaults(const Defaults& d);

private:
    PokemonDatabase() = default;
    ~PokemonDatabase();
    PokemonDatabase(const PokemonDatabase&)            = delete;
    PokemonDatabase& operator=(const PokemonDatabase&) = delete;

    void createTables();
    void initFixedSlots();
    void loadWildAndParty();

    PokemonState rowToPokemon(const QSqlQuery& q);
    void         writePokemonToRow(QSqlQuery& q, const PokemonState& p);
    void         dbWriteWild(const PokemonState& p);
    void         dbWritePartySlot(int slot, const PokemonState& p);
    void         dbWritePCSlot(int box, int slot, const PokemonState& p);
    void         dbDeletePCSlot(int box, int slot);

    std::pair<int, int> firstFreePC();

    bool        m_initialized   = false;
    bool        m_inMenuSession = false;
    int         m_saveId        = 1;
    std::string m_dbPath;

    PokemonState                                                m_wild;
    std::array<PokemonState, PARTY_SIZE>                        m_party;
    std::unordered_map<int, std::array<PokemonState, BOX_SIZE>> m_boxCache;

    PokemonState* cachePtr(int box, int slot);
};

#endif

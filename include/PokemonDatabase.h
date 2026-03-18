#pragma once

#include <PokeTypes.h>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <optional>
#include <QSqlQuery>

constexpr int PARTY_SIZE  = 6;
constexpr int BOX_SIZE    = 16;   // 4x4
constexpr int MAX_BOXES   = 99;

struct PokemonState {
    int  _id        = -1;
    int  pokedex_id = 0;
    int  variant_id = 0;
    int  pokeball_id= 0;
    std::string name;
    int  lvl        = 1;
    int  currentXP  = 0;
    Nature nature   = Nature::Hardy;
    int  moves[4]   = {0,0,0,0};
    bool hasExpShare= false;

    bool empty() const { return pokedex_id == 0; }
};

struct GameState {
    int  save_id         = 1;
    int  player_sprite_id= 0;
    std::string name     = "Player";
    int  current_box     = 0;
    int  unlocked_boxes  = 10;
};

// Pending change for deferred menu writes
struct PendingSlotChange {
    enum Kind { PartySet, PartySwap, PCSet, PCSwap, PCToParty, PartyToPC };
    Kind kind;
    int  a_box=-1, a_slot=-1;
    int  b_box=-1, b_slot=-1;
    std::optional<PokemonState> data; // for Set operations
};

class PokemonDatabase {
public:
    static PokemonDatabase& instance();

    bool initialize(const std::string& dbPath = "", int save_id = 1);
    void shutdown();

    // --- Wild (always cached) ---
    const PokemonState&                     wild()                     const { return m_wild; }
    void                                    setWild(const PokemonState& p);
    void                                    clearWild();

    // --- Party (always cached) ---
    const std::array<PokemonState,PARTY_SIZE>& party()                 const { return m_party; }
    void                                    setPartySlot(int slot, const PokemonState& p);
    void                                    clearPartySlot(int slot);
    void                                    swapPartySlots(int a, int b);

    // --- PC (demand-loaded, box-granular) ---
    void                                    loadBox(int box);
    void                                    prefetchBox(int box);       // non-blocking hint
    bool                                    isBoxLoaded(int box)       const;
    const std::array<PokemonState,BOX_SIZE>& getBox(int box)           const; // asserts loaded
    void                                    setPCSlot(int box, int slot, const PokemonState& p);
    void                                    swapPCSlots(int boxA, int slotA, int boxB, int slotB);

    // --- Catch during gameplay (immediate PC write) ---
    // Places caught pokemon in first available box/slot. Returns {box, slot} or {-1,-1}.
    std::pair<int,int>                      catchWildPokemon(int pokeball_id);

    // --- Game state ---
    GameState                               loadGameState();
    bool                                    saveGameState(const GameState& state);

    // --- Menu session (deferred writes) ---
    void                                    beginMenuSession();
    void                                    queuePartyChange(int slot, const PokemonState& p);
    void                                    queuePartySwap(int slotA, int slotB);
    void                                    queuePCChange(int box, int slot, const PokemonState& p);
    void                                    queuePCSwap(int boxA, int slotA, int boxB, int slotB);
    void                                    queuePCToParty(int box, int slot, int partySlot);
    void                                    queuePartyToPC(int partySlot, int box, int slot);
    void                                    commitMenuSession();
    void                                    rollbackMenuSession();

    // --- Exp share (gameplay utility) ---
    bool                                    toggleExpShare(int partySlot);
    std::vector<int>                        partyExpShareSlots();

    // --- Multi-save ---
    std::vector<GameState>                  listSaves();
    bool                                    switchSave(int save_id);

private:
    PokemonDatabase() = default;
    ~PokemonDatabase();
    PokemonDatabase(const PokemonDatabase&) = delete;
    PokemonDatabase& operator=(const PokemonDatabase&) = delete;

    void createTables();
    void initFixedSlots();
    void loadWildAndParty();

    PokemonState rowToPokemon(const QSqlQuery& q);
    void         writePokemonToRow(QSqlQuery& q, const PokemonState& p, bool bindSlotParams);
    void         dbWriteWild(const PokemonState& p);
    void         dbWritePartySlot(int slot, const PokemonState& p);
    void         dbWritePCSlot(int box, int slot, const PokemonState& p);
    void         dbDeletePCSlot(int box, int slot);
    std::pair<int,int> firstFreePC();

    bool                                          m_initialized = false;
    int                                           m_saveId      = 1;
    std::string                                   m_dbPath;

    // Caches
    PokemonState                                  m_wild;
    std::array<PokemonState, PARTY_SIZE>          m_party;
    std::unordered_map<int,
        std::array<PokemonState, BOX_SIZE>>       m_boxCache;

    // Menu session
    bool                                          m_inMenuSession = false;
    std::vector<PendingSlotChange>                m_pendingChanges;
    // Snapshot for rollback
    PokemonState                                  m_wildSnapshot;
    std::array<PokemonState, PARTY_SIZE>          m_partySnapshot;
};

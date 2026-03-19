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
    int  scale       = 2;
    int  speed       = 1;
    int  lvlRangeUp  = 5;
    int  lvlRangeDown= 5;
    bool expShareOn  = false;
};

class PokemonDatabase {
public:
    static PokemonDatabase& instance();

    bool initialize(const QString& dbPath = QString(), int save_id = 1);
    void shutdown();

    // ── Wild slot ────────────────────────────────────────────────────────────
    const PokemonState& wild() const { return m_wild; }
    void setWild(const PokemonState& p);
    void clearWild();

    // ── Party ─────────────────────────────────────────────────────────────────
    const std::array<PokemonState, PARTY_SIZE>& party() const { return m_party; }
    void setPartySlot(int slot, const PokemonState& p);
    int  partySize()           const;
    int  firstFreePartySlot()  const;

    // ── PC boxes ──────────────────────────────────────────────────────────────
    // Returns false on DB error; always check before using getBox().
    bool                                      loadBox(int box);
    bool                                      isBoxLoaded(int box) const;
    const std::array<PokemonState, BOX_SIZE>& getBox(int box)      const;
    bool                                      setPCSlot(int box, int slot, const PokemonState& p);
    bool                                      swapPCSlots(int boxA, int slotA, int boxB, int slotB);

    // ── Catching ──────────────────────────────────────────────────────────────
    std::pair<int, int> catchWildPokemon(int pokeball_id);

    // ── Save file ─────────────────────────────────────────────────────────────
    GameState loadGameState();
    bool      saveGameState(const GameState& state);

    // ── Menu session (transactional drag-and-drop) ────────────────────────────
    // beginMenuSession returns false if the DB transaction could not be opened;
    // do not call swap/commit/rollback in that case.
    bool beginMenuSession();
    bool swapByPos(int boxX, int slotX, int boxY, int slotY);
    bool commitMenuSession();
    bool rollbackMenuSession();

    // ── Defaults / settings ───────────────────────────────────────────────────
    bool toggleExpShare();
    bool isExpShareOn() const;

    // ── Multi-save management ─────────────────────────────────────────────────
    std::vector<GameState> listSaves();
    bool                   switchSave(int save_id);

    // ── In-place Pokémon edits ────────────────────────────────────────────────
    // All return false if the slot is empty or the DB write fails.
    bool renamePokemon  (int box, int slot, const std::string& newName);
    bool setPokemonMoves(int box, int slot, const int moves[4]);
    bool setPokemonMove (int box, int slot, int moveIndex, int moveId);

    // ── Defaults persistence ──────────────────────────────────────────────────
    Defaults loadDefaults();
    bool     writeDefaults(const Defaults& d);

private:
    PokemonDatabase() = default;
    ~PokemonDatabase();
    PokemonDatabase(const PokemonDatabase&)            = delete;
    PokemonDatabase& operator=(const PokemonDatabase&) = delete;

    // Schema
    bool createTables();
    bool initFixedSlots();

    // Bulk load
    bool loadWildAndParty();

    // Row ↔ struct helpers
    PokemonState rowToPokemon     (const QSqlQuery& q);
    void         writePokemonToRow(QSqlQuery& q, const PokemonState& p);

    // Low-level DB writes (return false on error)
    bool dbWriteWild      (const PokemonState& p);
    bool dbWritePartySlot (int slot, const PokemonState& p);
    bool dbWritePCSlot    (int box, int slot, const PokemonState& p);
    bool dbDeletePCSlot   (int box, int slot);

    std::pair<int, int> firstFreePC();

    // In-memory cache pointer; loads the box if needed. Returns nullptr on error
    // or out-of-range indices.
    PokemonState* cachePtr(int box, int slot);

    bool        m_initialized   = false;
    bool        m_inMenuSession = false;
    int         m_saveId        = 1;
    QString m_dbPath;

    PokemonState                                                m_wild;
    std::array<PokemonState, PARTY_SIZE>                        m_party;
    std::unordered_map<int, std::array<PokemonState, BOX_SIZE>> m_boxCache;
};

#endif


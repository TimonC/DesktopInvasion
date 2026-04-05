#ifndef POKEMONDATABASE_H
#define POKEMONDATABASE_H
#include <PokeTypes.h>
#include <string>
#include <array>
#include <unordered_map>
#include <vector>
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
    bool petModeOn    = false;
};

struct EligibleEntry { int level; int move_id; };

class PokemonDatabase {

public:
    static PokemonDatabase& instance();
    void clearCache();
    int  initialize();
    void shutdown();
    int  currentSaveId() const { return m_saveId; }
    bool setCurrentSaveId(int save_id);
    std::vector<std::pair<int, std::string>> listTrainerNames();
    int              createNewSave(const GameState& state, const PokemonState& starter);
    bool             deleteSave(int save_id);
    std::vector<int> listSaveIds();
    const PokemonState& wild() const { return m_wild; }
    void setWild(const PokemonState& p, int tmGet, int ballGet, int whichBall);
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
    std::array<int, 3> loadPokeballs();
    bool               changePokeball(int delta, int row);
    bool               addTechnicalMove(int moveId);
    bool               hasTechnicalMove(int moveId);
    std::array<int, 3> loadRewards();
    std::vector<int>  getTechnicalMoveList();
    std::vector<int>   filterKnownTMs(const std::vector<int>& moveIds);
    std::vector<EligibleEntry> filterKnownTMs(const std::vector<EligibleEntry>& entries);
    bool evolvePokemon(int boxIndex, int slot, int targetPokedexId, std::string name);

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
    int  nextSaveIdFromCounter();
    PokemonState rowToPokemon     (const QSqlQuery& q);
    void         writePokemonToRow(QSqlQuery& q, const PokemonState& p);
    bool dbWriteWild     (const PokemonState& p);
    bool dbSetRewards(int tmGet, int ballGet, int whichBall);
    bool dbWritePartySlot(int slot, const PokemonState& p);
    bool dbWritePCSlot   (int box, int slot, const PokemonState& p);
    bool dbDeletePCSlot  (int box, int slot);
    std::pair<int, int> firstFreePC();
    PokemonState*       cachePtr(int box, int slot);
    bool    m_initialized = false;
    int     m_saveId      = 0;
    QString m_dbPath;
    PokemonState                                                m_wild;
    std::array<PokemonState, PARTY_SIZE>                        m_party;
    std::unordered_map<int, std::array<PokemonState, BOX_SIZE>> m_boxCache;
};

#endif

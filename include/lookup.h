#ifndef LOOKUP_H
#define LOOKUP_H

#include <data_poke_asset.h>
#include <data_poke.h>
#include <data_move.h>
#include <data_poke_flavor.h>

//All accessing of pregenerated Pokemon data happens
//through the Lookup namespace.
//
//This data is generated using Python scripts that create
//static arrays that can then be indexed based on their ID.
//This "hardcoded" approach is suitable for this project,
//and is nice and cache-friendly.
//
//All the arrays are "extern const", and originally I had planned
//to access them directly whenever they were used. However, at some point
//I decided to add Lookup as a wrapper, for clarity, central bounds checking,
//and to avoid mistakes in accessing very large global arrays.
//
//If I were to refactor I would probably make the Python scripts write to one
//giant file with all the arrays without "extern const", and then expose global
//lookup methods in that file's header. That would be cleaner seperation.
//However, the Python scripts and the data structures were the result of
//many months of iteration, and the current approach were the result of that,
//and it's not really worth a refactor at this point. I simply wrote the code
//as if the arrays were not extern const, and only used the Lookup to access it.



struct PokeRoll{
    const int poke_id;
    const int tmId;
    const int ballCount;
    const int ballId;
};

namespace Lookup {
    const int MAX_POKEDEX_ID = 493;

    inline const Move* getMove(int moveId) {
        return (moveId >= 0 && moveId <= kMaxMoveId) ? kMovesByIndex[moveId] : nullptr;
    }

    inline const Poke* getPoke(int pokeDexId) {
        return (pokeDexId >= 0 && pokeDexId <= MAX_POKEDEX_ID) ? kPokesByIndex[pokeDexId] : nullptr;
    }

    inline const AssetInfo* getSpriteInfo(int pokeDexId) {
        return (pokeDexId >= 1 && pokeDexId <= MAX_POKEDEX_ID) ? kAssetInfo[pokeDexId - 1] : nullptr;
    }

    const PokeRoll weightedSamplePokemon(int pokemonLvl, const std::vector<int>& unavailableTmIds, std::mt19937 &rng);

    Nature getRandomNature(std::mt19937 &rng);

    QString getRandomFlavorText(int pokeDexId, std::mt19937 &rng);
}
#endif

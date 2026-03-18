#ifndef LOOKUP_H
#define LOOKUP_H

#include <data_poke_asset.h>
#include <data_poke.h>
#include <data_move.h>
#include <data_poke_flavor.h>
#include <cassert>

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

    inline QString getRandomFlavorText(int pokeDexId, std::mt19937& rng) {
        static const std::string_view* const FLAVOR_ARRAYS[16] = {
            kPokeFlavor_yellow, kPokeFlavor_red, kPokeFlavor_blue,
            kPokeFlavor_gold, kPokeFlavor_silver, kPokeFlavor_crystal,
            kPokeFlavor_ruby, kPokeFlavor_sapphire, kPokeFlavor_emerald,
            kPokeFlavor_firered, kPokeFlavor_leafgreen,
            kPokeFlavor_diamond, kPokeFlavor_pearl, kPokeFlavor_platinum,
            kPokeFlavor_heartgold, kPokeFlavor_soulsilver
        };

        static std::uniform_int_distribution<int> dists[4] = {
            std::uniform_int_distribution<int>(0, 15),
            std::uniform_int_distribution<int>(3, 15),
            std::uniform_int_distribution<int>(6, 15),
            std::uniform_int_distribution<int>(11, 15)
        };

        int gen = (pokeDexId > 386) ? 3 : (pokeDexId > 251) ? 2 : (pokeDexId > 151) ? 1 : 0;
        int gameId = dists[gen](rng);

        const std::string_view& flavor = FLAVOR_ARRAYS[gameId][pokeDexId];
        assert(!flavor.empty() && "Flavor text lookup should never return empty.");

        return QString::fromUtf8(flavor.data(), flavor.size());
    }
}

#endif

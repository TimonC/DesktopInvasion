#ifndef LOOKUP_H
#define LOOKUP_H

#include <data_poke_asset.h>
#include <data_poke.h>
#include <data_move.h>
#include <data_poke_flavor.h>

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

    inline QString getFlavorText(int pokeDexId, int gameId) {
        static const std::string_view* const FLAVOR_ARRAYS[28] = {
            kPokeFlavoryellow, kPokeFlavorred, kPokeFlavorblue,
            kPokeFlavorgold, kPokeFlavorsilver, kPokeFlavorcrystal,
            kPokeFlavorruby, kPokeFlavorsapphire, kPokeFlavoremerald,
            kPokeFlavorfirered, kPokeFlavorleafgreen, kPokeFlavordiamond,
            kPokeFlavorpearl, kPokeFlavorplatinum, kPokeFlavorheartgold,
            kPokeFlavorsoulsilver, kPokeFlavorblack, kPokeFlavorwhite,
            kPokeFlavorblack_2, kPokeFlavorwhite_2, kPokeFlavorx,
            kPokeFlavory, kPokeFlavoromega_ruby, kPokeFlavoralpha_sapphire,
            kPokeFlavorsword, kPokeFlavorshield, kPokeFlavorlets_go_pikachu,
            kPokeFlavorlets_go_eevee
        };

        const std::string_view& flavor = FLAVOR_ARRAYS[gameId][pokeDexId];
        return QString::fromUtf8(flavor.data(), flavor.size());
    }
}

#endif

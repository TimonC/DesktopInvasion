#ifndef LOOKUP_H
#define LOOKUP_H

#include <data_poke_asset.h>
#include <data_poke.h>
#include <data_move.h>
#include <data_poke_flavor.h>
#include <cassert>

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

    inline int getRandomTM_uniform(std::mt19937& rng){
        static std::uniform_int_distribution<int> tmDist(0, kTmCount);
        return kAllTmIds[tmDist(rng)];
    }

    inline const PokeRoll getRandomPokemonByCatchRate(int pokemonLvl, const std::vector<int>& unavailableTmIds, std::mt19937& rng) {
        static std::uniform_int_distribution<int> pokemonDist(1, kTotalCatchRateWeight);

        while (true) {
            int roll = pokemonDist(rng);
            int low = 1, high = 493;
            while (low < high) {
                int mid = (low + high) / 2;
                if (kCatchRateCumulativeWeights[mid] >= roll)
                    high = mid;
                else
                    low = mid + 1;
            }
            const Poke* poke = getPoke(low);

            int threshold = static_cast<int>(255.0 * std::exp(-0.11 * pokemonLvl));
            threshold = std::clamp(threshold, 3, 255);

            if (poke->catch_rate >= threshold) {
                int excess = poke->catch_rate - threshold;
                int range = 255 - threshold;
                int chancePercent = (range > 0) ? (excess * 100 / range) : 100;

                static std::uniform_int_distribution<int> rewardDist(1, 100);
                bool giveReward = (rewardDist(rng) <= chancePercent);

                int tmId = 0;
                int ballCount = 0;
                int ballType = 0;

                if (giveReward) {
                    std::vector<int> availableTmIds;
                    for (int i = 0; i < kTmCount; i++) {
                        if (std::find(unavailableTmIds.begin(), unavailableTmIds.end(), kAllTmIds[i]) == unavailableTmIds.end()) {
                            availableTmIds.push_back(kAllTmIds[i]);
                        }
                    }
                    static std::uniform_int_distribution<int> tmOrBallDist(1, 2);
                    bool isTm = (tmOrBallDist(rng) == 1) && !availableTmIds.empty();

                    if (isTm) {
                        std::uniform_int_distribution<int> dist(0, (int)availableTmIds.size() - 1);
                        tmId = availableTmIds[dist(rng)];
                    } else {
                        static std::uniform_int_distribution<int> typeDist(1, 100);
                        int typeRoll = typeDist(rng);
                        if (typeRoll <= 70)
                            ballType = 1;
                        else if (typeRoll <= 95)
                            ballType = 2;
                        else
                            ballType = 3;

                        static std::uniform_int_distribution<int> countDist(1, 3);
                        ballCount = countDist(rng);
                    }
                }

                return PokeRoll{ low, tmId, ballCount, ballType };
            }
        }
    }
}


#endif

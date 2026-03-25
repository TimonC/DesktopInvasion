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
        static std::vector<std::vector<int>> cachedValidPokemon;
        static std::vector<std::vector<int>> cachedCumulativeWeights;
        static std::vector<int> cachedTotalWeights;
        static bool initialized = false;

        if (!initialized) {
            cachedValidPokemon.resize(101);
            cachedCumulativeWeights.resize(101);
            cachedTotalWeights.resize(101);

            for (int level = 1; level <= 100; level++) {
                std::vector<int> validPokemon;
                std::vector<int> cumulativeWeights;
                int totalWeight = 0;

                for (int pokeId = 1; pokeId <= 493; pokeId++) {
                    const Poke* poke = getPoke(pokeId);

                    bool isEligible = false;
                    int parentLevel = kEvolutionParentLevel[pokeId];

                    if (parentLevel == -1) {
                        isEligible = true;
                    } else if (parentLevel <= level) {
                        isEligible = true;
                    }

                    if (isEligible) {
                        validPokemon.push_back(pokeId);
                        totalWeight += poke->catch_rate;
                        cumulativeWeights.push_back(totalWeight);
                    }
                }

                cachedValidPokemon[level] = validPokemon;
                cachedCumulativeWeights[level] = cumulativeWeights;
                cachedTotalWeights[level] = totalWeight;
            }
            initialized = true;
        }

        int level = pokemonLvl;
        if (level > 100) level = 100;
        if (level < 1) level = 1;

        int selectedId = 1;

        if (!cachedValidPokemon[level].empty()) {
            int totalWeight = cachedTotalWeights[level];
            std::uniform_int_distribution<int> dist(1, totalWeight);
            int rollNum = dist(rng);

            const auto& cumulativeWeights = cachedCumulativeWeights[level];
            int low = 0;
            int high = cumulativeWeights.size() - 1;

            while (low < high) {
                int mid = (low + high) / 2;
                if (cumulativeWeights[mid] >= rollNum) {
                    high = mid;
                } else {
                    low = mid + 1;
                }
            }

            selectedId = cachedValidPokemon[level][low];
        }

        const Poke* poke = getPoke(selectedId);

        int threshold = static_cast<int>(255.0 * std::exp(-0.11 * pokemonLvl));
        threshold = std::clamp(threshold, 3, 255);

        int deficit = threshold - poke->catch_rate;
        int range = threshold - 1;
        int qualityPercent = (range > 0) ? (deficit * 100 / range) : 0;
        qualityPercent = std::clamp(qualityPercent, 0, 100);

        static std::uniform_int_distribution<int> rewardTypeDist(1, 100);
        int rewardType = rewardTypeDist(rng);

        int tmId = 0;
        int ballCount = 0;
        int ballType = 0;

        if (rewardType <= 5) {
            std::vector<int> availableTmIds;
            for (int i = 0; i < kTmCount; i++) {
                if (std::find(unavailableTmIds.begin(), unavailableTmIds.end(), kAllTmIds[i]) == unavailableTmIds.end()) {
                    availableTmIds.push_back(kAllTmIds[i]);
                }
            }

            if (!availableTmIds.empty()) {
                std::uniform_int_distribution<int> dist(0, (int)availableTmIds.size() - 1);
                tmId = availableTmIds[dist(rng)];
            }
        }
        else if (rewardType <= 50) {
            static std::uniform_int_distribution<int> tierDist(1, 1000);
            int tierRoll = tierDist(rng);

            struct Reward {
                int ballType;
                int count;
            };

            std::vector<Reward> tiers;

            if (qualityPercent <= 20) {
                tiers = {{1, 1}, {1, 1}, {1, 1}, {1, 2}, {1, 2}, {1, 3}};
            } else if (qualityPercent <= 40) {
                tiers = {{1, 1}, {1, 1}, {1, 1}, {1, 2}, {1, 2}, {1, 3}, {2, 1}};
            } else if (qualityPercent <= 60) {
                tiers = {{1, 1}, {1, 1}, {1, 1}, {1, 2}, {1, 2}, {1, 3}, {2, 1}, {2, 1}};
            } else if (qualityPercent <= 80) {
                tiers = {{1, 1}, {1, 1}, {1, 1}, {1, 2}, {1, 2}, {1, 3}, {2, 1}, {2, 1}, {2, 2}};
            } else {
                tiers = {{1, 1}, {1, 1}, {1, 1}, {1, 2}, {1, 2}, {1, 3}, {2, 1}, {2, 1}, {2, 2}, {2, 3}, {3, 1}};
            }

            int tierIndex = (tierRoll * tiers.size() - 1) / 1000;
            tierIndex = std::clamp(tierIndex, 0, (int)tiers.size() - 1);

            ballType = tiers[tierIndex].ballType;
            ballCount = tiers[tierIndex].count;
        }

        return PokeRoll{ selectedId, tmId, ballCount, ballType };
    }

}
#endif

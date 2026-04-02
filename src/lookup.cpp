#include <lookup.h>

namespace Lookup {

// Calculates a weight for a given catch rate at a given level.
// This weight determines both the likelihood of a pokemon with that
// catch rate to spawn at that level, and also the quality of the
// reward that the pokemon drops upon defeat. Higher weight means less rare.
//
// At higher levels, the weight is simply the catch rate.
// At lower levels, the exponential decay affects lower-catch rate
// (i.e., rarer) Pokemon more strongly, so that they won't appear
// as often (or never when truncated to zero).
static int calculatePokeWeight(int catchRate, int level) {
    static constexpr double DECAY_RATE = 0.1;
    int threshold = static_cast<int>(255.0 * std::exp(-DECAY_RATE * level));
    threshold = std::max(threshold, 1);

    if (catchRate >= threshold) {
        return catchRate;
    }
    return catchRate * catchRate / threshold;
}

const PokeRoll weightedSamplePokemon(int pokemonLvl, const std::vector<int>& unavailableTmIds, std::mt19937& rng) {
    static std::array<std::vector<int>, 101> precomputedValidPokemon;
    static std::array<std::vector<int>, 101> precomputedCumulativeWeights;
    static std::array<int, 101> precomputedTotalWeights;
    static bool initialized = false;

    /* When first called, for each level a distribution is computed
       where the pokeWeight determines the range of each valid pokemon.
       This distribution can then be used for weighted sampling of valid pokemon. */
    if (!initialized) {
        for (int level = 1; level <= 100; ++level) {
            std::vector<int> validPokemon;
            std::vector<int> cumulativeWeights;
            int totalWeight = 0;

            for (int pokeId = 1; pokeId <= 493; ++pokeId) {
                /* A Pokemon is valid if it isn't the result of an evolution
                   that happens at a higher level than the given pokemonLvl.,
                   nor has a higher evolution below that level. */
                const Poke* poke = getPoke(pokeId);
                bool isEligible = false;
                bool noEvolves = false;
                int parentLevel = kEvolutionParentLevel[pokeId];
                int eligibleEvolveLevel = -1;
                if(poke->eligible_evolve_count>0){
                    eligibleEvolveLevel = poke->eligible_evolves[0].level;
                }

                if (parentLevel == -1) {
                    isEligible = true;
                    if(eligibleEvolveLevel == -1){
                        noEvolves = true;
                    }
                } else if (parentLevel <= level) {
                    isEligible = true;
                }

                if (isEligible) {
                    int weight = calculatePokeWeight(poke->catch_rate, level);
                    if (eligibleEvolveLevel != -1 && eligibleEvolveLevel <= level){
                        weight = weight / 5; //five times less weight for above-evolution levels
                    }else if(noEvolves){
                        weight = weight / 2; //half the weight for no-evolution pokemon
                    }
                    if (weight > 0) {
                        validPokemon.push_back(pokeId);
                        totalWeight += weight;
                        cumulativeWeights.push_back(totalWeight);
                    }
                }
            }

            precomputedValidPokemon[level] = std::move(validPokemon);
            precomputedCumulativeWeights[level] = std::move(cumulativeWeights);
            precomputedTotalWeights[level] = totalWeight;
        }
        initialized = true;
    }

    int level = std::clamp(pokemonLvl, 1, 100);
    int selectedId = 1;
    if (!precomputedValidPokemon[level].empty()) {
        int totalWeight = precomputedTotalWeights[level];
        std::uniform_int_distribution<int> dist(1, totalWeight);
        int rollNum = dist(rng);

        const auto& cumulativeWeights = precomputedCumulativeWeights[level];
        int low = 0, high = cumulativeWeights.size() - 1;

        //Binary search to get the pokedex_id from the distribution
        while (low < high) {
            int mid = (low + high) / 2;
            if (cumulativeWeights[mid] >= rollNum)
                high = mid;
            else
                low = mid + 1;
        }
        selectedId = precomputedValidPokemon[level][low];
    } else {
        qWarning() << "Spawn weight distributions failed to initialize";
    }

    /* After selecting the Pokemon, derive its "quality percent"
     of its weight based on the min and max weight at that level.
     This determines the quality of reward drops.

     There's always:
        -65% chance: no reward
        -30% chance: ball reward, with quality based on quality percent
        -5%  chance: a new TM
    */
    const int tmRewardBound = 5;
    const int ballRewardBound = 30 + tmRewardBound;

    const Poke* poke = getPoke(selectedId);
    int weight = calculatePokeWeight(poke->catch_rate, pokemonLvl);
    int minWeight = calculatePokeWeight(1, pokemonLvl);  //rarest
    int maxWeight = calculatePokeWeight(255, pokemonLvl); //most common
    int qualityPercent = std::clamp((maxWeight - weight) * 100 / (maxWeight - minWeight), 0, 100);

    static std::uniform_int_distribution<int> rewardTypeDist(1, 100);
    int rewardType = rewardTypeDist(rng);


    int tmId = 0, ballCount = 0, ballType = 0;
    if (rewardType <= tmRewardBound) {
        std::vector<int> availableTmIds;
        for (int i = 0; i < kTmCount; ++i) {
            if (std::find(unavailableTmIds.begin(), unavailableTmIds.end(), kAllTmIds[i]) == unavailableTmIds.end()) {
                availableTmIds.push_back(kAllTmIds[i]);
            }
        }
        if (!availableTmIds.empty()) {
            std::uniform_int_distribution<int> dist(0, (int)availableTmIds.size() - 1);
            tmId = availableTmIds[dist(rng)];
        }
    } else if (rewardType <= ballRewardBound) {
        static std::uniform_int_distribution<int> tierDist(1, 1000);
        int tierRoll = tierDist(rng);

        struct Reward { int ballType; int count; };
        std::vector<Reward> tiers;

        if (qualityPercent <= 20) {
            tiers = {{1,1}, {1,1}, {1,1}};                 // Great Ball x1
        } else if (qualityPercent <= 40) {
            tiers = {{1,1}, {1,1}, {1,2}};                 // Great Ball x1/x2
        } else if (qualityPercent <= 60) {
            tiers = {{1,1}, {1,2}, {2,1}};                 // Great & Ultra
        } else if (qualityPercent <= 80) {
            tiers = {{1,2}, {2,1}, {2,2}};                 // More Ultra
        } else if (qualityPercent <= 94) {
            tiers = {{2,1}, {2,2}, {2,3}};                 // Ultra only
        } else { // 95-100
            tiers = {{3,1}};                               // Master Ball
        }

        int tierIndex = (tierRoll * static_cast<int>(tiers.size()) - 1) / 1000;
        tierIndex = std::clamp(tierIndex, 0, static_cast<int>(tiers.size()) - 1);
        ballType = tiers[tierIndex].ballType;
        ballCount = tiers[tierIndex].count;
    }

    return PokeRoll{selectedId, tmId, ballCount, ballType};
}

QString getRandomFlavorText(int pokeDexId, std::mt19937& rng) {
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

    Nature getRandomNature(std::mt19937 &rng){
       static std::uniform_int_distribution<int> dist(0, 24);
       return static_cast<Nature>(dist(rng));
    }

}

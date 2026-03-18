#include "tests.h"
#include <globals.h>
#include <iostream>

void runAllTests() {
    testPokespawnRandomness();
}

void testPokespawnRandomness() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::map<int, int> counts;
    const int TOTAL_TRIALS = 100000;

    for (int i = 0; i < TOTAL_TRIALS; i++) {
        auto* pokemon = Globals::getPokemonInfo(std::nullopt);
        counts[pokemon->pokedexId]++;
    }

    // Check expected missing Pokémon
    int missingIds[] = {
        208,  // Steelix
        249,  // Lugia
        250,  // Ho-Oh
        321,  // Wailord
        382,  // Kyogre
        383,  // Groudon
        384,  // Rayquaza
        483,  // Dialga
        484,  // Palkia
        486,  // Regigigas
        487,  // Giratina
        493   // Arceus
    };
    for (int id : missingIds) {
        if (counts[id] > 0) {
            std::cout << "FAIL: ID " << id << " appeared\n";
            assert(false);
        }
    }


    int minCount = TOTAL_TRIALS, maxCount = 0;
    int minId = 0, maxId = 0;

    for (const auto& [id, count] : counts) {
        if (count < minCount && count > 0) minCount = count, minId = id;
        if (count > maxCount) maxCount = count, maxId = id;
    }

    std::cout << "PASS\n";
    std::cout << "Min: " << minCount << " (ID " << minId << ")\n";
    std::cout << "Max: " << maxCount << " (ID " << maxId << ")\n";
}

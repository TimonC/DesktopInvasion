#include "tests.h"
#include <globals.h>
#include <iostream>
#include <ctime>
#include <map>
#include <cassert>
#include <variant_mapper.h>

void runAllTests() {
    testPokespawnRandomness();
    testVariantMapper();
}

void testPokespawnRandomness() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::map<int, int> counts;
    const int TOTAL_TRIALS = 100000;

    for (int i = 0; i < TOTAL_TRIALS; i++) {
        auto* pokemon = Globals::getPokemonInfo();
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
void testVariantMapper() {

    // Basic 0-based test
    assert(VariantMapper::pokedexID2IconID(1, 0) == 0);    // Bulbasaur
    assert(VariantMapper::pokedexID2IconID(25, 0) == 24);  // Pikachu

    // Unown
    assert(VariantMapper::pokedexID2IconID(201, 0) == 200);   // (201-1) + 0 = 200
    assert(VariantMapper::pokedexID2IconID(201, 27) == 227);  // 200 + 27

    // Deoxys
    // Shift before Deoxys: Unown(27)
    assert(VariantMapper::pokedexID2IconID(386, 0) == (386-1) + 27);      // 385 + 27 = 412
    assert(VariantMapper::pokedexID2IconID(386, 3) == (386-1) + 27 + 3);  // 385 + 27 + 3 = 415

    // Rotom
    // Shift before Rotom: Unown(27) + Deoxys(3) + Burmy(2) + Wormadam(2) + Shellos(1) + Gastrodon(1) = 36
    assert(VariantMapper::pokedexID2IconID(478, 0) == (478-1) + 36);      // 477 + 36 = 513
    assert(VariantMapper::pokedexID2IconID(478, 5) == (478-1) + 36 + 5);  // 477 + 36 + 5 = 518

    // Giratina (NEW!)
    // Shift before Giratina: Unown(27) + Deoxys(3) + Burmy(2) + Wormadam(2) + Shellos(1) + Gastrodon(1) + Rotom(5) = 41
    assert(VariantMapper::pokedexID2IconID(487, 0) == (487-1) + 41);      // 486 + 41 = 527
    assert(VariantMapper::pokedexID2IconID(487, 1) == (487-1) + 41 + 1);  // 486 + 41 + 1 = 528

    // Shaymin
    // Shift before Shaymin: add Giratina's 1 = 42
    assert(VariantMapper::pokedexID2IconID(492, 0) == (492-1) + 42);      // 491 + 42 = 533
    assert(VariantMapper::pokedexID2IconID(492, 1) == (492-1) + 42 + 1);  // 491 + 42 + 1 = 534

    // Pokémon after all
    // Total shift: 42 + Shaymin's 1 = 43
    assert(VariantMapper::pokedexID2IconID(500, 0) == (500-1) + 43);      // 499 + 43 = 542

    // Check direct method matches
    assert(VariantMapper::pokedexID2IconID(487, 0) == VariantMapper::pokedexID2IconIDDirect(487, 0));
    assert(VariantMapper::pokedexID2IconID(487, 1) == VariantMapper::pokedexID2IconIDDirect(487, 1));

    std::cout << "All VariantMapper tests PASSED! Total extra variants: "
              << VariantMapper::getTotalExtraVariants() << "\n";
}

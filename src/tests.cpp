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

    // Test basic non-variant
    assert(VariantMapper::pokedexID2IconID(25, 0) == 25);
    assert(VariantMapper::pokedexID2IconID(1, 0) == 1);

    // Test Unown variants
    assert(VariantMapper::pokedexID2IconID(201, 0) == 201);
    assert(VariantMapper::pokedexID2IconID(201, 5) == 206);
    assert(VariantMapper::pokedexID2IconID(201, 27) == 228);

    // Test pokemon between Unown and Burmy
    assert(VariantMapper::pokedexID2IconID(300, 0) == 327);  // 300 + 27

    // Test Burmy variants
    assert(VariantMapper::pokedexID2IconID(412, 0) == 439);  // 412 + 27
    assert(VariantMapper::pokedexID2IconID(412, 1) == 440);
    assert(VariantMapper::pokedexID2IconID(412, 2) == 441);


    // Test Wormadam variants (413)
    // Unown(27) + Burmy(2) = 29
    assert(VariantMapper::pokedexID2IconID(413, 0) == 442);  // 413 + 29
    assert(VariantMapper::pokedexID2IconID(413, 1) == 443);
    assert(VariantMapper::pokedexID2IconID(413, 2) == 444);

    // Test pokemon between Wormadam and Shellos (420)
    // Unown(27) + Burmy(2) + Wormadam(2) = 31
    assert(VariantMapper::pokedexID2IconID(420, 0) == 451);  // 420 + 31

    // Test Shellos variants (422)
    // Unown(27) + Burmy(2) + Wormadam(2) = 31
    assert(VariantMapper::pokedexID2IconID(422, 0) == 453);  // 422 + 31
    assert(VariantMapper::pokedexID2IconID(422, 1) == 454);

    // Test Gastrodon variants (423)
    // Unown(27) + Burmy(2) + Wormadam(2) + Shellos(1) = 32
    assert(VariantMapper::pokedexID2IconID(423, 0) == 455);  // 423 + 32
    assert(VariantMapper::pokedexID2IconID(423, 1) == 456);

    // Test pokemon between Gastrodon and Rotom (450)
    // Unown(27) + Burmy(2) + Wormadam(2) + Shellos(1) + Gastrodon(1) = 33
    assert(VariantMapper::pokedexID2IconID(450, 0) == 483);  // 450 + 33

    // Test Rotom variants (478)
    // All before Rotom: 27 + 2 + 2 + 1 + 1 = 33
    assert(VariantMapper::pokedexID2IconID(478, 0) == 511);  // 478 + 33
    assert(VariantMapper::pokedexID2IconID(478, 1) == 512);
    assert(VariantMapper::pokedexID2IconID(478, 5) == 516);

    // Test pokemon between Rotom and Shaymin (485)
    // All before 485: 27 + 2 + 2 + 1 + 1 + 5 = 38
    assert(VariantMapper::pokedexID2IconID(485, 0) == 523);  // 485 + 38

    // Test Shaymin variants (492)
    // All before Shaymin: 27 + 2 + 2 + 1 + 1 + 5 = 38
    assert(VariantMapper::pokedexID2IconID(492, 0) == 530);  // 492 + 38
    assert(VariantMapper::pokedexID2IconID(492, 1) == 531);

    // Test pokemon after all variants (500)
    // Total shift: 27 + 2 + 2 + 1 + 1 + 5 + 1 = 39
    assert(VariantMapper::pokedexID2IconID(500, 0) == 539);  // 500 + 39

    std::cout << "All VariantMapper tests PASSED!\n";
}

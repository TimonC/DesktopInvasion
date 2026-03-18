#include "tests.h"
#include <globals.h>
#include <ctime>
#include <map>
#include <qdebug.h>
#include <cassert>
#include <PokeMath.h>

void runAllTests() {
    /* testPokespawnRandomness(); */
    testPokeMath();
}

void testPokeMath() {

//https://bulbapedia.bulbagarden.net/wiki/Stat //lvl 78 w/ Adamant Garchomp example
    int baseStats[] = {108, 130, 95, 80, 85, 102};
    int ivs[] = {24, 12, 30, 16, 23, 5};
    int evs[] = {74, 190, 91, 48, 84, 23};
    std::array<int, 6> garchomp = PokeMath::calculatePokeStats(
        78, baseStats, ivs, evs, {110, 100, 90, 100, 100}
    );
    int expected[] = {289, 278, 193, 135, 171, 171};
    bool allPass = true;
    for (int i = 0; i < 6; i++) {
        if (garchomp[i] != expected[i]) {
            qDebug() << "FAIL: Stat" << i << "got" << garchomp[i] << "expected" << expected[i];
            allPass = false;
        }
    }
    int xp = PokeMath::xpToNextLevel(78);
    if (xp != 18487) {
        qDebug() << "FAIL: xpToNextLevel(78) got" << xp << "expected 18487";
        allPass = false;
    }
    int exp = PokeMath::calculateExperience(50, 1, 184);
    if (exp != 1971) {
        qDebug() << "FAIL: calculateExperience got" << exp << "expected 1971";
        allPass = false;
    }
    int mod = PokeMath::applyStatModifier(100, 2);
    if (mod != 200) {
        qDebug() << "FAIL: applyStatModifier got" << mod << "expected 200";
        allPass = false;
    }
    if (PokeMath::calculatePoisonDamage(100) != 12) {
        qDebug() << "FAIL: calculatePoisonDamage";
        allPass = false;
    }
    if (PokeMath::calculateBurnDamage(100) != 12) {
        qDebug() << "FAIL: calculateBurnDamage";
        allPass = false;
    }
    if (PokeMath::calculateParalysisSpeed(100) != 50) {
        qDebug() << "FAIL: calculateParalysisSpeed";
        allPass = false;
    }
    if (PokeMath::calculateModifiedAccuracy(80, 1) != 106) {
        qDebug() << "FAIL: calculateModifiedAccuracy";
        allPass = false;
    }
    if (PokeMath::calculateDrain(50, 50) != 25) {
        qDebug() << "FAIL: calculateDrain";
        allPass = false;
    }
    if (PokeMath::calculateHeal(200, 50) != 100) {
        qDebug() << "FAIL: calculateHeal";
        allPass = false;
    }
    if (allPass) {
        qDebug() << "PokeMath PASS";
    }
}

void testPokespawnRandomness() {
    /* std::srand(static_cast<unsigned>(std::time(nullptr))); */

    /* std::map<int, int> counts; */
    /* const int TOTAL_TRIALS = 100000; */

    /* for (int i = 0; i < TOTAL_TRIALS; i++) { */
    /*     auto* pokemon = Globals::getPokemonInfo(); */
    /*     counts[pokemon->pokedexId]++; */
    /* } */

    /* // Check expected missing Pokémon */
    /* int missingIds[] = { */
    /*     208,  // Steelix */
    /*     249,  // Lugia */
    /*     250,  // Ho-Oh */
    /*     321,  // Wailord */
    /*     382,  // Kyogre */
    /*     383,  // Groudon */
    /*     384,  // Rayquaza */
    /*     483,  // Dialga */
    /*     484,  // Palkia */
    /*     486,  // Regigigas */
    /*     487,  // Giratina */
    /*     493   // Arceus */
    /* }; */
    /* for (int id : missingIds) { */
    /*     if (counts[id] > 0) { */
    /*         qDebug() << "FAIL: ID " << id << " appeared"; */
    /*         assert(false); */
    /*     } */
    /* } */


    /* int minCount = TOTAL_TRIALS, maxCount = 0; */
    /* int minId = 0, maxId = 0; */

    /* for (const auto& [id, count] : counts) { */
    /*     if (count < minCount && count > 0) minCount = count, minId = id; */
    /*     if (count > maxCount) maxCount = count, maxId = id; */
    /* } */

    /* qDebug() << "Pokemon Random Info PASS"; */
    /* qDebug() << "Min: " << minCount << " (ID " << minId << ")"; */
    /* qDebug() << "Max: " << maxCount << " (ID " << maxId << ")"; */
}

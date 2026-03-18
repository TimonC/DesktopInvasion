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

/* //https://bulbapedia.bulbagarden.net/wiki/stat //lvl 78 w/ adamant garchomp example */
/*     int basestats[] = {108, 130, 95, 80, 85, 102}; */
/*     int ivs[] = {24, 12, 30, 16, 23, 5}; */
/*     int evs[] = {74, 190, 91, 48, 84, 23}; */
/*     std::array<int, 6> garchomp = pokemath::calculatepokestats( */
/*         78, basestats, ivs, evs, {110, 100, 90, 100, 100} */
/*     ); */
/*     int expected[] = {289, 278, 193, 135, 171, 171}; */
/*     bool allpass = true; */
/*     for (int i = 0; i < 6; i++) { */
/*         if (garchomp[i] != expected[i]) { */
/*             qdebug() << "fail: stat" << i << "got" << garchomp[i] << "expected" << expected[i]; */
/*             allpass = false; */
/*         } */
/*     } */
/*     int xp = pokemath::xptonextlevel(78); */
/*     if (xp != 18487) { */
/*         qdebug() << "fail: xptonextlevel(78) got" << xp << "expected 18487"; */
/*         allpass = false; */
/*     } */
/*     int exp = pokemath::calculateexperience(50, 1, 184); */
/*     if (exp != 1971) { */
/*         qdebug() << "fail: calculateexperience got" << exp << "expected 1971"; */
/*         allpass = false; */
/*     } */
/*     int mod = pokemath::applystatmodifier(100, 2); */
/*     if (mod != 200) { */
/*         qdebug() << "fail: applystatmodifier got" << mod << "expected 200"; */
/*         allpass = false; */
/*     } */
/*     if (pokemath::calculatepoisondamage(100) != 12) { */
/*         qdebug() << "fail: calculatepoisondamage"; */
/*         allpass = false; */
/*     } */
/*     if (pokemath::calculateburndamage(100) != 12) { */
/*         qdebug() << "fail: calculateburndamage"; */
/*         allpass = false; */
/*     } */
/*     if (pokemath::calculateparalysisspeed(100) != 50) { */
/*         qdebug() << "fail: calculateparalysisspeed"; */
/*         allpass = false; */
/*     } */
/*     if (pokemath::calculatemodifiedaccuracy(80, 1) != 106) { */
/*         qdebug() << "fail: calculatemodifiedaccuracy"; */
/*         allpass = false; */
/*     } */
/*     if (pokemath::calculatedrain(50, 50) != 25) { */
/*         qdebug() << "fail: calculatedrain"; */
/*         allpass = false; */
/*     } */
/*     if (pokemath::calculateheal(200, 50) != 100) { */
/*         qdebug() << "fail: calculateheal"; */
/*         allPass = false; */
/*     } */
/*     if (allPass) { */
/*         qDebug() << "PokeMath PASS"; */
/*     } */
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

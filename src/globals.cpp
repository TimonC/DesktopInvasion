#include "globals.h"
#include "Player.h"
#include <QGuiApplication>
#include <qwindow.h>

Player& getPlayer() {
    static std::unique_ptr<Player> player = std::make_unique<Player>();
    return *player;
}

// Get a random Pokemon by pokedex ID
const PokemonInfo* getRandomPokemon() {
    // Get all unique pokedex IDs
    QSet<int> availablePokedexIds;
    for (int i = 0; i < kPokemonCount; ++i) {
        availablePokedexIds.insert(kPokemonList[i].pokedexId);
    }

    // Convert to list for random access
    QList<int> pokedexIdList = availablePokedexIds.values();

    if (pokedexIdList.isEmpty()) {
        qFatal("No available Pokemon pokedex IDs found!");
    }

    // Pick random pokedex ID
    int randomIndex = std::rand() % pokedexIdList.size();
    int randomPokedexId = pokedexIdList[randomIndex];

    return findPokemonByPokedexId(randomPokedexId);
}
const PokemonInfo* findPokemonByPokedexId(int pokedexId) {
    for (int i = 0; i < kPokemonCount; ++i) {
        if (kPokemonList[i].pokedexId == pokedexId) {
            return &kPokemonList[i]; // return pointer to the entry
        }
    }
    qFatal("Pokemon with spriteId %d not found!", pokedexId);
    return nullptr; // this line won't be reached
}



QRect screenSize(){
    static QRect availableScreen = QGuiApplication::primaryScreen()->availableGeometry();
    return availableScreen;
}

#include "globals.h"
#include "Player.h"
#include <QGuiApplication>
#include <QWindow>
#include <cstdlib>

bool DEBUG = false;

const QRect& screenGeometry() {
    static const QRect geometry = QGuiApplication::primaryScreen()->availableGeometry();
    return geometry;
}

Player& getPlayer() {
    static std::unique_ptr<Player> player = std::make_unique<Player>();
    return *player;
}

const PokemonInfo* getRandomPokemon() {
    // Get all unique pokedex IDs
    QSet<int> availablePokedexIds;
    for (int i = 0; i < kPokemonCount; ++i) {
        availablePokedexIds.insert(kPokemonList[i].pokedexId);
    }

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
            return &kPokemonList[i];
        }
    }
    qFatal("Pokemon with pokedexId %d not found!", pokedexId);
    return nullptr;
}

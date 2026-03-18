#include "globals.h"
#include "Player.h"
#include <QGuiApplication>
#include <QHash>
#include <cstdlib>
#include <cassert>

namespace Globals {
    bool DEBUG = false;

    const QRect& screenGeometry() {
        static const QRect geometry = QGuiApplication::primaryScreen()->availableGeometry();
        return geometry;
    }

    Player& getPlayer() {
        static Player player;
        return player;
    }

    const PokemonInfo* getRandomPokemon() {
        static QHash<int, const PokemonInfo*> lookup = [](){
            QHash<int, const PokemonInfo*> map;
            for (int i = 0; i < kPokemonCount; ++i) {
                map[kPokemonList[i].pokedexId] = &kPokemonList[i];
            }
            return map;
        }();

        QList<int> ids = lookup.keys();
        assert(!ids.isEmpty());
        int randomIndex = std::rand() % ids.size();
        return lookup[ids[randomIndex]];
    }

    const PokemonInfo* findPokemonByPokedexId(int pokedexId) {
        static QHash<int, const PokemonInfo*> lookup = [](){
            QHash<int, const PokemonInfo*> map;
            for (int i = 0; i < kPokemonCount; ++i) {
                map[kPokemonList[i].pokedexId] = &kPokemonList[i];
            }
            return map;
        }();

        auto it = lookup.constFind(pokedexId);
        assert(it != lookup.constEnd());
        return it.value();
    }
}

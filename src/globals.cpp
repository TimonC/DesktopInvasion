#include "globals.h"
#include "Player.h"
#include <QGuiApplication>
#include <QHash>
#include <QScreen>
#include <pokemon_data.h>
namespace Globals {
    bool DEBUG = false;
    int SCALE = 3;
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

    const PokemonInfo* getPokemonByPokedexId(int pokedexId) {
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

  const SpriteInfo* getSpriteInfo(int spriteId, int generation) {
        // Build lookup table on first call
        static QHash<QPair<int, int>, const SpriteInfo*> lookup = [](){
            QHash<QPair<int, int>, const SpriteInfo*> map;
            for (int i = 0; i < kSpriteCount; ++i) {
                QPair<int, int> key(kSpriteList[i].spriteId, kSpriteList[i].generation);
                map[key] = &kSpriteList[i];
            }
            return map;
        }();

        QPair<int, int> key(spriteId, generation);
        auto it = lookup.constFind(key);
        if (it != lookup.constEnd()) {
            return it.value();
        }
        return nullptr;
    }

    QSize getSpriteSize(int spriteId, int generation) {
        const SpriteInfo* info = getSpriteInfo(spriteId, generation);
        if (info) {
            return QSize(info->max_width, info->max_height);
        }
        return QSize(0, 0); // Return invalid size if not found
    }
}

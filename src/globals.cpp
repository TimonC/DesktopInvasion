#include <globals.h>
#include <QGuiApplication>
#include <unordered_map>
#include <QScreen>
#include <QRect>
#include <QSize>

namespace Globals {
    bool DEBUG = false;
    int SCALE = 3;
    int POKE_PADDING = 2;
    const int MAX_POKEDEX_ID = 493;

    const QRect& screenGeometry() {
        static const QRect geometry = QGuiApplication::primaryScreen()->geometry();
        return geometry;
    }

    const PokemonInfo* getPokemonInfo(int pokedexId) {
        static std::unordered_map<int, const PokemonInfo*> lookup;
        static std::vector<const PokemonInfo*> pokemonVector;

        if (lookup.empty()) {
            for (int i = 0; i < kPokemonCount; ++i) {

                int id = kPokemonList[i].pokedexId;
                if(lookup.find(id)==lookup.end()){
                    lookup[id] = &kPokemonList[i];
                    pokemonVector.push_back(&kPokemonList[i]);
                }

            }
        }

        if (pokedexId>0) {
            if (pokedexId > MAX_POKEDEX_ID) {
                assert(!"Max pokedex id is ${MAX_POKEDEX_ID}");
            }

            auto it = lookup.find(pokedexId);
            if (it == lookup.end()) {
                assert(!"Pokemon ID not available in this version");
            }
            return it->second;
        }

        return pokemonVector[std::rand() % pokemonVector.size()];
    }

    const SpriteInfo* getSpriteInfo(int spriteId, int generation) {
        static std::unordered_map<int, const SpriteInfo*> lookup;

        if (lookup.empty()) {
            for (int i = 0; i < kSpriteCount; ++i) {
                int key = (kSpriteList[i].spriteId << 16) | kSpriteList[i].generation;
                lookup[key] = &kSpriteList[i];
            }
        }

        int key = (spriteId << 16) | generation;
        auto it = lookup.find(key);
        return it != lookup.end() ? it->second : nullptr;
    }

    QSize getSpriteSize(int spriteId, int generation) {
        const SpriteInfo* info = getSpriteInfo(spriteId, generation);
        if (info) {
            return QSize(info->max_width, info->max_height);
        }
        return QSize(0, 0); // Return invalid size if not found
    }
}

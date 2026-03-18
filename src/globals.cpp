#include <globals.h>
#include <QGuiApplication>
#include <QScreen>
#include <QRect>
#include <QSize>
#include <cassert>
#include <vector>

namespace Globals {
    bool DEBUG = false;
    int SCALE = 3;
    int POKE_PADDING = 2;
    const int MAX_POKEDEX_ID = 493;

    const QRect& screenGeometry() {
        static const QRect geometry = QGuiApplication::primaryScreen()->geometry();
        return geometry;
    }

    const Move* getMove(int moveId) {
        if(moveId < 0 || moveId > kMaxMoveId){
            return nullptr;
        }
        return kMovesByIndex[moveId];
    }

    const PokemonInfo* getPokemonInfo(int pokedexId) {
        static const PokemonInfo* pokeLookup[MAX_POKEDEX_ID + 1] = {};
        static std::vector<const PokemonInfo*> uniquePokemon;

        static bool initialized = false;
        if (!initialized) {
            for (int i = 0; i < kPokemonCount; ++i) {
                int id = kPokemonList[i].pokedexId;
                if (id >= 1 && id <= MAX_POKEDEX_ID && pokeLookup[id] == nullptr) {
                    pokeLookup[id] = &kPokemonList[i];
                    uniquePokemon.push_back(&kPokemonList[i]);
                }
            }
            initialized = true;
        }

        if (pokedexId <= 0) {
            return uniquePokemon[std::rand() % uniquePokemon.size()];
        }

        if (pokedexId > MAX_POKEDEX_ID) {
            assert(!"Max pokedex id is ${MAX_POKEDEX_ID}");
        }

        const PokemonInfo* result = pokeLookup[pokedexId];
        if (!result) {
            assert(!"Pokemon ID not available in this version");
        }
        return result;
    }

    const SpriteInfo* getSpriteInfo(int spriteId, int generation) {
        constexpr int MAX_SPRITE_ID = 1000;
        constexpr int MAX_GEN = 9;

        static const SpriteInfo* spriteLookup[MAX_SPRITE_ID][MAX_GEN] = {};

        static bool initialized = false;
        if (!initialized) {
            for (int i = 0; i < kSpriteCount; ++i) {
                int id = kSpriteList[i].spriteId;
                int gen = kSpriteList[i].generation;
                if (id >= 0 && id < MAX_SPRITE_ID &&
                    gen >= 1 && gen <= MAX_GEN) {
                    spriteLookup[id][gen - 1] = &kSpriteList[i];
                }
            }
            initialized = true;
        }

        if (spriteId >= 0 && spriteId < MAX_SPRITE_ID &&
            generation >= 1 && generation <= MAX_GEN) {
            return spriteLookup[spriteId][generation - 1];
        }

        return nullptr;
    }

    QSize getSpriteSize(int spriteId, int generation) {
        const SpriteInfo* info = getSpriteInfo(spriteId, generation);
        if (info) {
            return QSize(info->max_width, info->max_height);
        }
        return QSize(0, 0);
    }
}

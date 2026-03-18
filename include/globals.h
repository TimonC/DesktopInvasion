#ifndef GLOBALS_H
#define GLOBALS_H

#include <data_poke_asset.h>
#include <data_poke.h>
#include <data_move.h>
#include <QSize>

class QRect;

namespace Globals {
    extern bool DEBUG;
    extern int SCALE;
    extern int POKE_PADDING;
    extern const int MAX_POKEDEX_ID;

    const QRect& screenGeometry();

    inline const Move* getMove(int moveId) {
        return (moveId >= 0 && moveId <= kMaxMoveId) ? kMovesByIndex[moveId] : nullptr;
    }

    inline const Poke* getPoke(int pokeDexId) {
        return (pokeDexId >= 0 && pokeDexId <= MAX_POKEDEX_ID) ? kPokesByIndex[pokeDexId] : nullptr;
    }

    inline const AssetInfo* getSpriteInfo(int pokeDexId) {
        return (pokeDexId >= 1 && pokeDexId <= MAX_POKEDEX_ID) ? kAssetInfo[pokeDexId - 1] : nullptr;
    }

    inline QSize getSpriteSize(int pokeDexId) {
        if (const AssetInfo* info = getSpriteInfo(pokeDexId)) {
            return QSize(info->width, info->height);
        }
        return QSize(0, 0);
    }
}

#endif

#include "variant_mapper.h"
#include <array>

namespace {
    constexpr int MAX_ICON_ID = 1000;

    constexpr int kVariantList[][2] = {
        {201, 28}, {386, 4}, {412, 3}, {413, 3},
        {422, 2}, {423, 2}, {478, 6}, {487, 2}, {492, 2},
    };

    constexpr int kVariantCount = 9;

    // Compute base icon table with 0-based icons
    static std::array<int, MAX_ICON_ID + 1> computeBaseIconTable() {
        std::array<int, MAX_ICON_ID + 1> table{};
        int shift = 0;
        int idx = 0;

        for (int id = 1; id <= MAX_ICON_ID; ++id) {
            // Pokedex IDs are 1-based, icon IDs are 0-based: icon = (id - 1) + shift
            table[id] = (id - 1) + shift;

            if (idx < kVariantCount && id == kVariantList[idx][0]) {
                shift += (kVariantList[idx][1] - 1);
                idx++;
            }
        }
        return table;
    }
}

int VariantMapper::pokedexID2IconID(int pokedexId, int variantId) {
    if (pokedexId < 1 || pokedexId > MAX_ICON_ID) return -1;
    if (variantId < 0) variantId = 0;

    // Get base icon from precomputed table
    static const std::array<int, MAX_ICON_ID + 1> baseTable = computeBaseIconTable();
    int baseIcon = baseTable[pokedexId];

    // Check if this Pokemon has variants
    for (int i = 0; i < kVariantCount; ++i) {
        if (kVariantList[i][0] == pokedexId) {
            if (variantId < kVariantList[i][1]) {
                return baseIcon + variantId;
            }
            break;  // Invalid variant ID, return base
        }
    }

    return baseIcon;  // No variants or invalid variantId
}

int VariantMapper::pokedexID2IconIDDirect(int pokedexId, int variantId) {
    if (pokedexId < 1 || pokedexId > MAX_ICON_ID) return -1;
    if (variantId < 0) variantId = 0;

    // Calculate shift
    int shift = 0;
    for (int i = 0; i < kVariantCount; ++i) {
        if (kVariantList[i][0] < pokedexId) {
            shift += (kVariantList[i][1] - 1);
        } else {
            break;
        }
    }

    // Pokedex IDs are 1-based, icon IDs are 0-based
    int baseIconId = (pokedexId - 1) + shift;

    // Check for variants
    for (int i = 0; i < kVariantCount; ++i) {
        if (kVariantList[i][0] == pokedexId) {
            if (variantId < kVariantList[i][1]) {
                return baseIconId + variantId;
            }
            break;
        }
    }

    return baseIconId;
}

int VariantMapper::getVariantCount(int pokedexId) {
    if (pokedexId < 1 || pokedexId > MAX_ICON_ID) return 0;

    for (int i = 0; i < kVariantCount; ++i) {
        if (kVariantList[i][0] == pokedexId) {
            return kVariantList[i][1];
        }
    }

    return 1;
}

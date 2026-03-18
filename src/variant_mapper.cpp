#include "variant_mapper.h"
#include <array>

namespace {
    constexpr int MAX_POKEDEX_ID = 1000;  // Changed from MAX_ICON_ID to match header

    constexpr int kVariantList[][2] = {
        {201, 28}, {412, 3}, {413, 3}, {422, 2},
        {423, 2}, {478, 6}, {492, 2},
    };

    constexpr int kVariantCount = 7;

    // Compute shift table - FIXED LOGIC!
    static std::array<int, MAX_POKEDEX_ID + 1> computeShiftTable() {
        std::array<int, MAX_POKEDEX_ID + 1> table{};
        int shift = 0;
        int idx = 0;

        for (int id = 1; id <= MAX_POKEDEX_ID; ++id) {
            // Store current shift BEFORE checking if this ID adds more
            table[id] = shift;

            // If this ID has variants, add them to shift for NEXT IDs
            if (idx < kVariantCount && id == kVariantList[idx][0]) {
                shift += (kVariantList[idx][1] - 1);
                idx++;
            }
        }
        return table;
    }

    // Compute base icon table (no variant dimension needed)
    static std::array<int, MAX_POKEDEX_ID + 1> computeBaseIconTable() {
        std::array<int, MAX_POKEDEX_ID + 1> table{};
        int shift = 0;
        int idx = 0;

        for (int id = 1; id <= MAX_POKEDEX_ID; ++id) {
            table[id] = id + shift;

            if (idx < kVariantCount && id == kVariantList[idx][0]) {
                shift += (kVariantList[idx][1] - 1);
                idx++;
            }
        }
        return table;
    }
}

const std::array<int, VariantMapper::MAX_POKEDEX_ID + 1>& VariantMapper::getShiftTable() {
    static const std::array<int, MAX_POKEDEX_ID + 1> table = computeShiftTable();
    return table;
}

// Remove getIconTable() - we don't need it anymore

int VariantMapper::pokedexID2IconID(int pokedexId, int variantId) {
    if (pokedexId < 1 || pokedexId > MAX_POKEDEX_ID) return -1;
    if (variantId < 0) variantId = 0;

    // Get base icon from precomputed table
    static const std::array<int, MAX_POKEDEX_ID + 1> baseTable = computeBaseIconTable();
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
    if (pokedexId < 1 || pokedexId > MAX_POKEDEX_ID) return -1;
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

    int baseIconId = pokedexId + shift;

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
    if (pokedexId < 1 || pokedexId > MAX_POKEDEX_ID) return 0;

    for (int i = 0; i < kVariantCount; ++i) {
        if (kVariantList[i][0] == pokedexId) {
            return kVariantList[i][1];
        }
    }

    return 1;
}

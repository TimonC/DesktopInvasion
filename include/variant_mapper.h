#ifndef VARIANT_MAPPER_H
#define VARIANT_MAPPER_H

#include <array>

class VariantMapper {
private:
    static constexpr int MAX_POKEDEX_ID = 1000;

    static constexpr int kVariantList[][2] = {
        {201, 28}, // unown (27 extra variants)
        {412, 3},  // burmy (2 extra)
        {413, 3},  // wormadam (2 extra)
        {422, 2},  // shellos (1 extra)
        {423, 2},  // gastrodon (1 extra)
        {478, 6},  // rotom (5 extra)
        {492, 2},  // shaymin (1 extra)
    };

    static constexpr int kVariantCount = 7;
    static constexpr int TOTAL_EXTRA_VARIANTS = 39; // 27 + 2 + 2 + 1 + 1 + 5 + 1

    // Static table getters
    static const std::array<int, MAX_POKEDEX_ID + 1>& getShiftTable();

public:
    VariantMapper() = delete;

    // Main conversion function using precomputed tables
    static int pokedexID2IconID(int pokedexId, int variantId = 0);

    // Direct calculation without tables
    static int pokedexID2IconIDDirect(int pokedexId, int variantId = 0);

    static int getVariantCount(int pokedexId);
    static int getTotalExtraVariants() { return TOTAL_EXTRA_VARIANTS; }
};

#endif // VARIANT_MAPPER_H
